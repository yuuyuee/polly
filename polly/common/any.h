// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_ANY_H_
#define OAK_COMMON_ANY_H_

#include <stdlib.h>
#include <initializer_list>
#include <typeinfo>
#include <type_traits>
#include <utility>

#include "oak/common/macros.h"

namespace oak {
// Exception thrown by the value-returning forms of any_cast
// on a type mismatch.
class bad_any_cast: public std::bad_cast {
 public:
  bad_any_cast() noexcept = default;
  virtual ~bad_any_cast() noexcept = default;

  virtual const char* what() const noexcept {
    return "Bad any cast";
  }
};

// Throw delegate
OAK_ATTR_NORETURN inline void ThrowBadAnyCast() {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
  throw bad_any_cast{};
#else
  abort();
#endif
}

class Any;

namespace any_internal {
template <typename>
void* AnyCastHelper(const Any*) noexcept;

// Avoid dynamic allocations for small objects, but such an optimization
// may only be applied to types for which std::is_nothrow_move_constructible
// returns true.
union Storage {
  constexpr Storage() noexcept: ptr(nullptr) {}

  // uses small-object optimization
  alignas(sizeof(void*)) char buffer[sizeof(void*)];
  // creates object on the heap
  void* ptr;
};

template <typename Tp>
struct IsSmallObject
    : public std::integral_constant<
        bool,
        sizeof(Tp) <= sizeof(void*) &&
        alignof(Tp) <= alignof(void*) &&
        std::is_nothrow_move_constructible<Tp>::value> {};

struct Args {
  void* obj;
  const std::type_info* typeinfo;
  Storage* storage;
};

enum Ops {
  GET, GET_TYPE_INFO, COPY, MOVE, DESTRUCT
};

// contained object
template <typename Tp, bool = IsSmallObject<Tp>::value>
struct Operators {
  // Constructors and destructor
  template <typename Up>
  static void Construct(Storage* storage, Up&& value) {
    storage->ptr = new Tp(std::forward<Up>(value));
  }

  template <typename... Args>
  static void Construct(Storage* storage, Args&&... args) {
    storage->ptr = new Tp(std::forward<Args>(args)...);
  }

  static void Operator(Ops ops, const Storage* self, Args* args) {
    auto ptr = reinterpret_cast<const Tp*>(self->ptr);
    switch (ops) {
    case Ops::GET:
      args->obj = const_cast<Tp*>(ptr);
      break;
    case Ops::GET_TYPE_INFO:
#if defined(__GXX_RTTI)  // gcc & clang
      args->typeinfo = &typeid(Tp);
#endif
      break;
    case Ops::COPY:
      args->storage->ptr = new Tp(*ptr);
      break;
    case Ops::MOVE:
      args->storage->ptr = const_cast<Tp*>(ptr);
      break;
    case Ops::DESTRUCT:
      delete ptr;
      break;
    }
  }
};

// in-place contained object
template <typename Tp>
struct Operators<Tp, true> {
  template <typename Up>
  static void Construct(Storage* storage, Up&& value) {
    ::new(storage->buffer) Tp(std::forward<Up>(value));
  }

  template <typename... Args>
  static void Construct(Storage* storage, Args&&... args) {
    ::new(storage->buffer) Tp(std::forward<Args>(args)...);
  }

  static void Operator(Ops ops, const Storage* self, Args* args) {
    auto ptr = reinterpret_cast<const Tp*>(self->buffer);
    switch (ops) {
    case Ops::GET:
      args->obj = const_cast<Tp*>(ptr);
      break;
    case Ops::GET_TYPE_INFO:
#if defined(__GXX_RTTI)  // gcc & clang
      args->typeinfo = &typeid(Tp);
#endif
      break;
    case Ops::COPY:
      ::new (&args->storage->buffer) Tp(*ptr);
      break;
    case Ops::MOVE:
      ::new (&args->storage->buffer) Tp(std::move(*const_cast<Tp*>(ptr)));
      ptr->~Tp();
      break;
    case Ops::DESTRUCT:
      ptr->~Tp();
      break;
    }
  }
};

template <typename Tp>
struct in_place_type_tag {
  in_place_type_tag() = delete;
  in_place_type_tag(const in_place_type_tag&) = delete;
  in_place_type_tag& operator=(const in_place_type_tag&) = delete;
};

template <typename...>
struct Conjunction;

template <>
struct Conjunction<>: public std::true_type {};

template <typename T1>
struct Conjunction<T1>: public T1 {};

template <typename T1, typename T2>
struct Conjunction<T1, T2>: public std::conditional<T1::value, T2, T1>::type {};

template <typename T1, typename T2, typename T3, typename... Tn>
struct Conjunction<T1, T2, T3, Tn...>
    : public std::conditional<
        T1::value,
        Conjunction<T2, T3, Tn...>,
        T1
      >::type {};

template <typename Tp>
struct Negation: public std::integral_constant<bool, !Tp::value> {};

template <bool Value, typename Tp = void>
using enable_if_t = typename std::enable_if<Value, Tp>::type;

template <typename... Tn>
using Requires = enable_if_t<Conjunction<Tn...>::value, bool>;

template <typename Tp>
using remove_cvref_t = typename std::remove_cv<
    typename std::remove_reference<Tp>::type>::type;
}  // namespace any_internal

template <typename Tp>
using in_place_type_t = void(*)(any_internal::in_place_type_tag<Tp>);

template <typename Tp>
void in_place_type(any_internal::in_place_type_tag<Tp>) {}

template <typename Tp>
struct is_in_place_type: public std::false_type {};

template <typename Tp>
struct is_in_place_type<in_place_type_t<Tp>>: public std::true_type {};

// The class any describes a type-safe container for single values of any type.
// An object of class any stores an instance of any type that satisfies the
// constructor requirements or is empty, and this is referred to as the state
// of the class any object. The stored instance is called the contained object.
// Two states are equivalent if they are either both empty or if both are not
// empty and if the contained objects are equivalent.
class Any {
 public:
  // Constructs an empty object.
  constexpr Any() noexcept: operator_(nullptr) {}

#define OAK_ANY_REQ_CTOR(Tp, args)        \
  any_internal::Requires<                 \
      std::is_copy_constructible<Tp>,     \
      std::is_constructible<Tp, args>     \
  > = true

  // Constructs an object with initial content an object of type Up.
  template <typename Tp, typename... Args,
            typename Up = typename std::decay<Tp>::type,
            OAK_ANY_REQ_CTOR(Up, Args...)>
  explicit Any(in_place_type_t<Tp>, Args&&... args)
      : operator_(any_internal::Operators<Up>::Operator) {
    any_internal::Operators<Up>::Construct(
        &storage_, std::forward<Args>(args)...);
  }

#define OAK_ANY_REQ_CTOR2(Tp, Il, args)   \
  any_internal::Requires<                 \
      std::is_copy_constructible<Tp>,     \
      std::is_constructible<Tp, Il, args> \
  > = true

  // Constructs an object with initial content an object of type Up.
  template <typename Tp, typename Up, typename... Args,
            typename Up2 = typename std::decay<Tp>::type,
            OAK_ANY_REQ_CTOR2(Up2, std::initializer_list<Up>, Args...)>
  explicit Any(in_place_type_t<Tp>,
               std::initializer_list<Up> il,
               Args&&... args)
      : operator_(any_internal::Operators<Up2>::Operator) {
    any_internal::Operators<Up2>::Construct(
        &storage_, il, std::forward<Args>(args)...);
  }

#define OAK_ANY_REQ_COPY_CTOR(Tp)                                 \
  any_internal::Requires<                                         \
      std::is_copy_constructible<Tp>,                             \
      any_internal::Negation<std::is_same<Tp, Any>>,              \
      any_internal::Negation<is_in_place_type<Tp>>  \
  > = true

  // Constructs an object with initial content an object of type VTp.
  template <typename Tp,
            typename Up = typename std::decay<Tp>::type,
            OAK_ANY_REQ_COPY_CTOR(Up)>
  Any(Tp&& value): operator_(any_internal::Operators<Up>::Operator) {
    any_internal::Operators<Up>::Construct(
        &storage_, std::forward<Tp>(value));
  }

  ~Any() { reset(); }

  // Copies content of other into a new instance, so that any content
  // is equivalent in both type and value to those of other prior
  // to the constructor call or empty if other is empty.
  Any(const Any& other) {
    if (other.has_value()) {
      any_internal::Args args;
      args.storage = &storage_;
      other.operator_(any_internal::Ops::COPY, &other.storage_, &args);
      operator_ = other.operator_;
    } else {
      operator_ = nullptr;
    }
  }

  // Moves content of other into a new instance, so that any conten
  // is equivalent in both type and value to those of other prior
  // to the constructor call or empty if other is empty.
  Any(Any&& other) noexcept {
    if (other.has_value()) {
      any_internal::Args args;
      args.storage = &storage_;
      other.operator_(any_internal::Ops::MOVE, &other.storage_, &args);
      operator_ = other.operator_;
      other.operator_ = nullptr;
    } else {
      operator_ = nullptr;
    }
  }

  // Assignment operators

  Any& operator=(const Any& rhs) {
    *this = Any(rhs);
    return *this;
  }

  Any& operator=(Any&& rhs) noexcept {
    if (&rhs != this) {
      reset();
      if (rhs.has_value()) {
        any_internal::Args args;
        args.storage = &storage_;
        rhs.operator_(any_internal::Ops::MOVE, &rhs.storage_, &args);
        operator_ = rhs.operator_;
        rhs.operator_ = nullptr;
      }
    }

    return *this;
  }

  template <typename Tp, OAK_ANY_REQ_COPY_CTOR(typename std::decay<Tp>::type)>
  Any& operator=(Tp&& rhs) {
    *this = Any(std::forward<Tp>(rhs));
    return *this;
  }

  // Modifiers

  // Change the contained object, constructing the new object directly.
  template <typename Tp,
            typename... Args,
            typename Up = typename std::decay<Tp>::type,
            OAK_ANY_REQ_CTOR(Up, Args...)>
  Up& emplace(Args&&... args) {
    reset();
    any_internal::Operators<Up>::Construct(
        &storage_, std::forward<Args>(args)...);
    operator_ = any_internal::Operators<Up>::Operator;
    any_internal::Args res;
    operator_(any_internal::Ops::GET, &storage_, &res);
    return *static_cast<Up*>(res.obj);
  }

  // Change the contained object, constructing the new object directly.
  template <typename Tp,
            typename Up,
            typename... Args,
            typename Up2 = typename std::decay<Tp>::type,
            OAK_ANY_REQ_CTOR2(Up2, std::initializer_list<Up>, Args...)>
  Up2& emplace(std::initializer_list<Up> il, Args&&... args) {
    reset();
    any_internal::Operators<Up2>::Construct(
        &storage_, il, std::forward<Args>(args)...);
    operator_ = any_internal::Operators<Up2>::Operator;
    any_internal::Args res;
    operator_(any_internal::Ops::GET, &storage_, &res);
    return *static_cast<Up2*>(res.obj);
  }

  // Destroys contained object.
  void reset() noexcept {
    if (has_value()) {
      operator_(any_internal::Ops::DESTRUCT, &storage_, nullptr);
      operator_ = nullptr;
    }
  }

  // Swaps two any objects.
  void swap(Any& other) noexcept {
    if (this == &other || (!has_value() && !other.has_value()))
      return;
    Any tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
  }

  // Observers

  // Checks if object holds a value.
  bool has_value() const noexcept {
    return operator_ != nullptr;
  }

#if defined(__GXX_RTTI)
  // Returns the typeid of the contained value.
  const std::type_info& type() const noexcept {
    if (has_value()) {
      any_internal::Args args;
      operator_(any_internal::Ops::GET_TYPE_INFO, &storage_, &args);
      return *args.typeinfo;
    }
    return typeid(void);
  }
#endif

 private:
  template <typename>
  friend void* any_internal::AnyCastHelper(const Any*) noexcept;
  using operator_fn = void (*)(any_internal::Ops,
                               const any_internal::Storage*,
                               any_internal::Args*);
  operator_fn operator_;
  any_internal::Storage storage_;
};

// Overloads the std::swap algorithm for any.
// Swaps the content of two any objects by calling lhs.swap(rhs)
inline void swap(Any& lhs, Any& rhs) noexcept {
  lhs.swap(rhs);
}

// Type-safe access to the contained object.
template <typename Tp>
Tp any_cast(const Any& operand) {
  using Up = any_internal::remove_cvref_t<Tp>;
  static_assert(std::is_constructible<Tp, const Up&>::value,
      "Template argument must be constructible from a const value.");
  auto res = any_cast<Up>(&operand);
  if (res)
    return static_cast<Tp>(*res);
  ThrowBadAnyCast();
}

template <typename Tp>
Tp any_cast(Any& operand) {
  using Up = any_internal::remove_cvref_t<Tp>;
  static_assert(std::is_constructible<Tp, Up&>::value,
      "Template argument must be constructible from an lvalue.");
  auto res = any_cast<Up>(&operand);
  if (res)
    return static_cast<Tp>(*res);
  ThrowBadAnyCast();
}

template <typename Tp>
Tp any_cast(Any&& operand) {
  using Up = any_internal::remove_cvref_t<Tp>;
  static_assert(std::is_constructible<Tp, Up>::value,
      "Template argument must be constructible from an rvalue.");
  auto res = any_cast<Up>(&operand);
  if (res)
    return static_cast<Tp>(std::move(*res));
  ThrowBadAnyCast();
}

namespace any_internal {
template <typename Tp>
void* AnyCastHelper(const Any* any) noexcept {
  using Up = remove_cvref_t<Tp>;
  constexpr bool flag =
      !std::is_same<Up, typename std::decay<Up>::type>::value ||
      !std::is_copy_constructible<Up>::value;
  if (flag)
    return nullptr;

  if (any->operator_ != any_internal::Operators<Up>::Operator)
    return nullptr;

#if defined(__GXX_RTTI)
  if (any->type() != typeid(Tp))
    return nullptr;
#endif

  any_internal::Args args;
  any->operator_(any_internal::Ops::GET, &any->storage_, &args);
  return args.obj;
}
}  // namespace any_internal

template <typename Tp>
inline Tp* any_cast(Any* operand) noexcept {
  return operand
      ? static_cast<Tp*>(any_internal::AnyCastHelper<Tp>(operand))
      : nullptr;
}

template <typename Tp>
inline const Tp* any_cast(const Any* operand) noexcept {
  return operand
      ? static_cast<Tp*>(any_internal::AnyCastHelper<Tp>(operand))
      : nullptr;
}

// Creates an any object.
template <typename Tp, typename... Args>
Any make_any(Args&&... args) {
  return Any(in_place_type<Tp>, std::forward<Args>(args)...);
}

template <typename Tp, typename Up, typename... Args>
Any make_any(std::initializer_list<Up> il, Args&&... args) {
  return Any(in_place_type<Tp>, il, std::forward<Args>(args)...);
}

}  // namespace oak

#endif  // OAK_COMMON_ANY_H_
