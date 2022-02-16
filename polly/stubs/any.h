#pragma once

#include "stubs/internal/config.h"

#if defined(POLLY_HAVE_STD_ANY)
#include <any>

namespace polly {
using std::any;
using std::any_cast;
using std::bad_any_cast;
using std::make_any;
} // namespace polly

#else // POLLY_HAVE_STD_ANY

#include <initializer_list>

#include "stubs/utility.h"
#include "stubs/internal/any.h"
#if !defined(POLLY_HAVE_EXCEPTIONS)
#include "stubs/internal/raw_logging.h"
#endif

namespace polly {
// Exception thrown by the value-returning forms of any_cast on a type mismatch.
class bad_any_cast
#ifdef POLLY_HAVE_RTTI
    : public std::bad_cast
#else
    : public std::exception
#endif //POLLY_HAVE_RTTI
{
public:
  bad_any_cast() noexcept {}
  virtual ~bad_any_cast() noexcept = default;

  virtual const char* what() const noexcept override {
    return "Bad any cast";
  }
};

// Throw delegate
[[noreturn]] inline void ThrowBadAnyCast() {
#ifndef POLLY_HAVE_EXCEPTIONS
  POLLY_RAW_LOG(FATAL, bad_any_cast{}.what());
#else
  throw bad_any_cast{};
#endif
}

class any;

namespace any_internal {
template <typename>
void* any_cast_helper(const any*) noexcept;
} // namespace any_internal

// The class any describes a type-safe container for single values of any type.
// An object of class any stores an instance of any type that satisfies the
// constructor requirements or is empty, and this is referred to as the state
// of the class any object. The stored instance is called the contained object.
// Two states are equivalent if they are either both empty or if both are not
// empty and if the contained objects are equivalent.
class any {
public:
  // Constructs an empty object.
  constexpr any() noexcept: operator_(nullptr) {}

#define POLLY_ANY_REQ_CTOR(Tp, args)  \
  Requires<                           \
      std::is_copy_constructible<Tp>, \
      std::is_constructible<Tp, args> \
  > = true

  // Constructs an object with initial content an object of type VTp.
  template <typename Tp,
            typename... Args,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_CTOR(VTp, Args...)>
  explicit any(in_place_type_t<Tp>, Args&&... args)
      : operator_(any_internal::Operators<VTp>::Operator) {
    any_internal::Operators<VTp>::Construct(&storage_, std::forward<Args>(args)...);
  }

#define POLLY_ANY_REQ_CTOR2(Tp, Il, args) \
  Requires<                               \
      std::is_copy_constructible<Tp>,     \
      std::is_constructible<Tp, Il, args> \
  > = true

  // Constructs an object with initial content an object of type VTp.
  template <typename Tp,
            typename Up,
            typename... Args,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_CTOR2(VTp, std::initializer_list<Up>, Args...)>
  explicit any(in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args)
      : operator_(any_internal::Operators<VTp>::Operator) {
    any_internal::Operators<VTp>::Construct(&storage_, il, std::forward<Args>(args)...);
  }

#define POLLY_ANY_REQ_COPY_CTOR(Tp)                 \
  Requires<                                         \
      std::is_copy_constructible<Tp>,               \
      negation<std::is_same<Tp, any>>,              \
      negation<any_internal::is_in_place_type<Tp>>  \
  > = true

  // Constructs an object with initial content an object of type VTp.
  template <typename Tp,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_COPY_CTOR(VTp)>
  any(Tp&& value): operator_(any_internal::Operators<VTp>::Operator) {
    any_internal::Operators<VTp>::Construct(&storage_, std::forward<Tp>(value));
  }

  ~any() { reset(); }

  // Copies content of other into a new instance, so that any content is equivalent
  // in both type and value to those of other prior to the constructor call,
  // or empty if other is empty.
  any(const any& other) {
    if (other.has_value()) {
      any_internal::Args args;
      args.storage = &storage_;
      other.operator_(any_internal::Ops::Copy, &other.storage_, &args);
      operator_ = other.operator_;
    } else {
      operator_ = nullptr;
    }
  }

  // Moves content of other into a new instance, so that any content is equivalent
  // in both type and value to those of other prior to the constructor call,
  // or empty if other is empty.
  any(any&& other) noexcept {
    if (other.has_value()) {
      any_internal::Args args;
      args.storage = &storage_;
      other.operator_(any_internal::Ops::Move, &other.storage_, &args);
      operator_ = other.operator_;
      other.operator_ = nullptr;
    } else {
      operator_ = nullptr;
    }
  }

  // Assignment operators

  any& operator=(const any& rhs) {
    *this = any(rhs);
    return *this;
  }

  any& operator=(any&& rhs) noexcept {
    if (&rhs != this) {
      reset();
      if (rhs.has_value()) {
        any_internal::Args args;
        args.storage = &storage_;
        rhs.operator_(any_internal::Ops::Move, &rhs.storage_, &args);
        operator_ = rhs.operator_;
        rhs.operator_ = nullptr;
      }
    }

    return *this;
  }

  template <typename Tp, POLLY_ANY_REQ_COPY_CTOR(typename std::decay<Tp>::type)>
  any& operator=(Tp&& rhs) {
    *this = any(std::forward<Tp>(rhs));
    return *this;
  }

  // Modifiers

  // Change the contained object, constructing the new object directly.
  template <typename Tp,
            typename... Args,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_CTOR(VTp, Args...)>
  VTp& emplace(Args&&... args) {
    reset();
    any_internal::Operators<VTp>::Construct(&storage_, std::forward<Args>(args)...);
    operator_ = any_internal::Operators<VTp>::Operator;
    any_internal::Args res;
    operator_(any_internal::Ops::Get, &storage_, &res);
    return *static_cast<VTp*>(res.obj);
  }

  // Change the contained object, constructing the new object directly.
  template <typename Tp,
            typename Up,
            typename... Args,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_CTOR2(VTp, std::initializer_list<Up>, Args...)>
  VTp& emplace(std::initializer_list<Up> il, Args&&... args) {
    reset();
    any_internal::Operators<VTp>::Construct(&storage_, il, std::forward<Args>(args)...);
    operator_ = any_internal::Operators<VTp>::Operator;
    any_internal::Args res;
    operator_(any_internal::Ops::Get, &storage_, &res);
    return *static_cast<VTp*>(res.obj);
  }

  // Destroys contained object.
  void reset() noexcept {
    if (has_value()) {
      operator_(any_internal::Ops::Destruct, &storage_, nullptr);
      operator_ = nullptr;
    }
  }

  // Swaps two any objects.
  void swap(any& other) noexcept {
    if (this == &other || (!has_value() && !other.has_value()))
      return;
    any tmp = std::move(*this);
    *this = std::move(other);
    other = std::move(tmp);
  }

  // Observers

  // Checks if object holds a value.
  bool has_value() const noexcept {
    return operator_ != nullptr;
  }

#ifdef POLLY_HAVE_RTTI
  // Returns the typeid of the contained value.
  const std::type_info& type() const noexcept {
    if (has_value()) {
      any_internal::Args args;
      operator_(any_internal::Ops::GetTypeInfo, &storage_, &args);
      return *args.typeinfo;
    }
    return typeid(void);
  }
#endif

private:
  template <typename>
  friend void* any_internal::any_cast_helper(const any*) noexcept;
  using operator_fn =
      void (*)(any_internal::Ops, const any_internal::Storage*, any_internal::Args*);
  operator_fn operator_;
  any_internal::Storage storage_;
};

// Overloads the std::swap algorithm for any.
// Swaps the content of two any objects by calling lhs.swap(rhs)
inline void swap(any& lhs, any& rhs) noexcept {
  lhs.swap(rhs);
}

// Type-safe access to the contained object.
template <typename Tp>
Tp any_cast(const any& operand) {
  using Up = remove_cvref_t<Tp>;
  static_assert(std::is_constructible<Tp, const Up&>::value,
      "Template argument must be constructible from a const value.");
  auto res = any_cast<Up>(&operand);
  if (res)
    return static_cast<Tp>(*res);
  ThrowBadAnyCast();
}

template <typename Tp>
Tp any_cast(any& operand) {
  using Up = remove_cvref_t<Tp>;
  static_assert(std::is_constructible<Tp, Up&>::value,
      "Template argument must be constructible from an lvalue.");
  auto res = any_cast<Up>(&operand);
  if (res)
    return static_cast<Tp>(*res);
  ThrowBadAnyCast();
}

template <typename Tp>
Tp any_cast(any&& operand) {
  using Up = remove_cvref_t<Tp>;
  static_assert(std::is_constructible<Tp, Up>::value,
      "Template argument must be constructible from an rvalue.");
  auto res = any_cast<Up>(&operand);
  if (res)
    return static_cast<Tp>(std::move(*res));
  ThrowBadAnyCast();
}

namespace any_internal {
template <typename Tp>
void* any_cast_helper(const any* any) noexcept {
  using Up = remove_cvref_t<Tp>;
  constexpr bool flag =
      !std::is_same<Up, typename std::decay<Up>::type>::value ||
      !std::is_copy_constructible<Up>::value;
  if (flag)
    return nullptr;

  if (any->operator_ != any_internal::Operators<Up>::Operator)
    return nullptr;

#ifdef POLLY_HAVE_RTTI
  if (any->type() != typeid(Tp))
    return nullptr;
#endif
  any_internal::Args args;
  any->operator_(any_internal::Ops::Get, &any->storage_, &args);
  return args.obj;
}
} // namespace any_internal

template <typename Tp>
inline Tp* any_cast(any* operand) noexcept {
  return operand
      ? static_cast<Tp*>(any_internal::any_cast_helper<Tp>(operand))
      : nullptr;
}

template <typename Tp>
inline const Tp* any_cast(const any* operand) noexcept {
  return operand
      ? static_cast<Tp*>(any_internal::any_cast_helper<Tp>(operand))
      : nullptr;
}

// Creates an any object.
template <typename Tp, typename... Args>
any make_any(Args&&... args) {
  return any(in_place_type<Tp>, std::forward<Args>(args)...);
}

template <typename Tp, typename Up, typename... Args>
any make_any(std::initializer_list<Up> il, Args&&... args) {
  return any(in_place_type<Tp>, il, std::forward<Args>(args)...);
}

} // namespace polly

#endif // POLLY_HAVE_STD_ANY
