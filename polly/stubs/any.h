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

#include "stubs/internal/any.h"
#if !defined(POLLY_HAVE_EXCEPTIONS)
#include "stubs/internal/raw_logging.h"
#endif
#include "stubs/internal/type_id.h"
#include "stubs/utility.h"
#include "stubs/type_traits.h"

namespace polly {
// Exception thrown by the value-returning forms of any_cast on a type mismatch.
class bad_any_cast: public std::bad_cast {
public:
  bad_any_cast() noexcept {}
  virtual ~bad_any_cast() noexcept = default;

  virtual const char* what() const noexcept override {
    return "Bad any cast";
  }
};

// Throw delegate
[[noreturn]] inline void ThrowBadAnyCast() {
#if !defined(POLLY_HAVE_EXCEPTIONS)
  POLLY_RAW_LOG(FATAL, bad_any_cast{}.what());
#else
  throw bad_any_cast{};
#endif
}

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

  template <typename Tp,
            typename... Args,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_CTOR(VTp, Args...)>
  explicit any(in_place_type_t<Tp>, Args&&... args)
      : operator_(&any_internal::Operator<VTp>::Operator) {
    any_internal::Construct(storage_, std::forward<Args>(args)...);
  }

#define POLLY_ANY_REQ_CTOR2(Tp, Il, args)  \
  Requires<                           \
      std::is_copy_constructible<Tp>, \
      std::is_constructible<Tp, Il, args> \
  > = true

  template <typename Tp,
            typename Up,
            typename... Args,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_CTOR2(VTp, std::initializer_list<Up>, Args...)>
  explicit any(in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args)
      : operator_(&any_internal::Operator<VTp>::Operator) {
    any_internal::Construct(storage_, il, std::forward<Args>(args)...);
  }

#define POLLY_ANY_REQ_COPY_CTOR(Tp)                 \
  Requires<                                         \
      std::is_copy_constructible<Tp>,               \
      negation<std::is_same<Tp, any>>,              \
      negation<any_internal::is_in_place_type<Tp>>  \
  > = true

  template <typename Tp,
            typename VTp = typename std::decay<Tp>::type,
            POLLY_ANY_REQ_TYPE_CHECK(VTp)>
  any(Tp&& value): operator_(&any_internal::Operators<VTp>::Operator) {
    any_internal::Operators<VTp>::Construct(storage_, std::forward<Tp>(value));
  }

  ~any() { reset(); }

  // Copies or moves content of other into a new instance, so that any content
  // is equivalent in both type and value to those of other prior to the
  // constructor call, or empty if other is empty.
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

  any& operator=(const any& rhs) {
    *this = any(rhs);
    return *this;
  }

  any& operator=(any&& rhs) noexcept {
    if (rhs.has_value()) {
      reset();
      Args args;
      args.storage = &storage_;
      rhs.operator_(any_internal::Ops::Move, &rhs.storage_, &args);
      operator_ = rhs.operator_;
      rhs.operator_ = nullptr;
    } else {
      reset();
    }
    return *this;
  }

  template <typename Tp, POLLY_ANY_REQ_COPY_CTOR(tyepname std::decay<Tp>::type)>
  any& operator=(Tp&& rhs) {
    *this = any(std::forward<Tp>(rhs));
    return *this;
  }

  // Modifiers
  template <typename Tp, typename... Args>
  typename std::decay<Tp>::type emplace(Args&&... args);

  template <typename Tp, typename Up, typename... Args>
  typename std::decay<Tp>::type emplace(std::initializer_list<Up> il, Args&&... args);

  void reset() noexcept {
    if (has_value()) {
      operator_(any_internal::Ops::Destruct, storage_, nullptr);
      operator_ = nullptr;
    }
  }

  void swap(any& other) noexcept;

  // Observers
  bool has_value() const noexcept {
    return operator_ != nullptr;
  }

#ifdef POLLY_HAVE_RTTI
  const std::type_info& type() const noexcept {
    if (has_value()) {
      Args args;
      operator_(any_internal::Ops::GetTypeInfo, nullptr, &args);
      return *args.typeinfo;
    }
    return typeid(void);
  }
#endif

private:
  using operator_fn =
      void (*)(any_internal::Ops, const any_internal::Storage*, any_internal::Args*);
  operator_fn operator_;
  any_internal::Storage storage_;
};

// Overloads the std::swap algorithm for any.
// Swaps the content of two any objects by calling lhs.swap(rhs)
void swap(any&, any&) noexcept;

template <typename Tp>
Tp any_cast(const any& operand);

template <typename Tp>
Tp any_cast(any& operand);

template <typename Tp>
Tp any_cast(any&& operand);

template <typename Tp>
Tp* any_cast(any* operand);

template <typename Tp>
const Tp* any_cast(const any* operand);

template <typename Tp, typename... Args>
any make_any(Args&&... args);

template <typename Tp, typename Up, typename... Args>
any make_any(std::initializer_list<Up> il, Args&&... args);

} // namespace polly

#endif // POLLY_HAVE_STD_ANY
