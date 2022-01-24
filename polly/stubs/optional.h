#pragma once

#include "stubs/internal/config.h"

#if defined(POLLY_HAVE_STD_OPTIONAL)
#include <optional>
#include <utility>

namespace polly {
using std::bad_optional_access;
using std::optional;
using std::make_optional;
using std::nullopt_t;
using std::nullopt;
using std::in_place_t;
using std::in_place;
} // namespace polly

#else // POLLY_HAVE_STD_OPTIONAL

#include "stubs/macros.h"
#include "stubs/internal/throw_delegate.h"
#include "stubs/internal/raw_logging.h"
#include "stubs/internal/optional.h"

namespace polly {
// A type of object to thrown by std::optional::value when accessing an optional
// object that does not contain a value.
class bad_optional_access : public std::exception {
 public:
  bad_optional_access() = default;
  virtual ~bad_optional_access() = default;

  virtual const char* what() const noexcept override {
    return "optional has no value";
  }
};

// Throw delegator
[[noreturn]] void ThrowBadOptionalAccess() {
#if !defined(POLLY_HAVE_EXCEPTIONS)
  POLLY_LOG_FATAL(bad_optional_access{}.what())
#else
  throw bad_optional_access{};
#endif
}

// nullopt_t
// An empty class type used to indicate optional type with uninitialized state.
// optional has a constructor with nullopt_t as a single argument, which create
// an optional that does not contain a value.
struct nullopt_t {
  struct construct_tag {};

  // constructor must be a non-aggregate literal type and cannot have a default
  // constructor or an initializer-list constructor. It must have a constexpr
  // constructor that takes a optional_internal::construct_tag.
  explicit constexpr nullopt_t(nullopt_t::construct_tag) {}
};

// nullopt
POLLY_INLINE_CONSTEXPR(nullopt_t, nullopt, nullopt_t::construct_tag{});

// class optional
// The class template optional manages an optional contained value, a value
// that may or may not be present.
// A common use cse for optional is the return value of a function that may
// fail. As opposed to other approaches, such as pair<T, bool>, optional
// handles expensive to construct objects well and is more readable, as the
// intent is expressed explicitly.
// There are no optional references, instantiates an optional with a reference
// type is ill-formed. Alternatively, an optional of a reference_wrapper of type
// T may be used to hold a reference. In addition, instantiates an optional
// with the tag types nullopt_t or in_palce_t is iill-formed.
template <typename Tp>
class optional
    : private optional_internal::optional_base<Tp>,
      private optional_internal::enable_copy_move<Tp, optional<Tp>> {
private:
  static_assert(!std::is_same<nullopt_t, typename std::remove_cv<Tp>::type>::value,
      "optional<nullopt_t> is not allowed.");
  static_assert(!std::is_same<in_place_t, typename std::remove_cv<Tp>::type>::value,
      "optional<in_place_t> is not allowed.");
  static_assert(!std::is_reference<Tp>::value, "optional<reference> is not allowed.");

  template <typename Up>
  using not_self = negation<std::is_same<optional, remove_cvref_t<Up>>>;

  template <typename Up>
  using not_in_place = negation<std::is_same<in_place_t, remove_cvref_t<Up>>>;

public:
  using value_type = Tp;

  // Constructs an optional object holding an empty value.
  constexpr optional() noexcept {}

  // Constructs an optional object with `nullopt` holding an empty value.
  constexpr optional(nullopt_t) noexcept {}

  // Copy constructor
  optional(const optional&) = default;

  // Move constructor
  optional(optional&&) = default;

  // Constructs an optional object direct initialized from `args`. in_place_t
  // is a tag used to indicate that the contained object should be constructed
  // in place.
  // The function does not participate in the overload resolution unless the
  // following conditions are met:
  // std::is_constructible<Tp, Args...>::value is true.
  template <typename... Args,
      Requires<std::is_constructible<Tp, Args...>> = true>
  constexpr explicit optional(in_place_t, Args&&... args)
      noexcept(std::is_nothrow_constructible<Tp, Args...>::value)
      : optional_internal::optional_base<Tp>(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args,
      Requires<std::is_constructible<Tp, std::initializer_list<Up>&, Args...>> = true>
  constexpr explicit optional(in_place_t, std::initializer_list<Up> il, Args&&... args)
      noexcept(std::is_nothrow_constructible<Tp, std::initializer_list<Up>&, Args...>::value)
      : optional_internal::optional_base<Tp>(in_place, il, std::forward<Args>(args)...) {}

  // Converting copy constructor conditionallly explicit.
  // The function does not participate in the overload resolution unless the
  // following conditions are met:
  // 1. std::is_constructible<Tp, const Up&>::value is true.
  // 2. Tp is not constructible or convertible from any expression of type
  //    (possibly const) std::optional<Up>, i.e, the following 8 type traits
  //    are all false:
  //      1) std::is_constructible<Tp, std::optional<Up>&>::value
  //      2) std::is_constructible<Tp, std::optional<Up>& const>::value
  //      3) std::is_constructible<Tp, std::optional<Up>&&>::value
  //      4) std::is_constructible<Tp, std::optional<Up>&& const>::value
  //      5) std::is_convertible<std::optional<Up>&, Tp>::value
  //      6) std::is_convertible<std::optional<Up>& const, Tp>::value
  //      7) std::is_convertible<std::optional<Up>&&, Tp>::value
  //      8) std::is_convertible<std::optional<Up>&& const, Tp>::value
  // This constructor is explicit if and only if std::is_convertible<const Up&, Tp>
  // is false.

#define POLLY_OPT_REQ_IMPLICIT_CONVERT_COPY_CTOR(Tp, Up)      \
  Requires<                                                   \
      negation<std::is_same<Tp, Up>>,                              \
      std::is_constructible<Tp, const Up&>,                   \
      std::is_convertible<const Up&, Tp>,                     \
      negation<optional_internal::converts_from_optional<Tp, Up>>  \
  > = true

#define POLLY_OPT_REQ_EXPLICIT_CONVERT_COPY_CTOR(Tp, Up)      \
  Requires<                                                   \
      negation<std::is_same<Tp, Up>>,                              \
      std::is_constructible<Tp, const Up&>,                   \
      negation<std::is_convertible<const Up&, Tp>>,                \
      negation<optional_internal::converts_from_optional<Tp, Up>>  \
  > = true

  // Converting copy constructor implicit.
  template <typename Up, POLLY_OPT_REQ_IMPLICIT_CONVERT_COPY_CTOR(Tp, Up)>
  optional(const optional<Up>& other)
      noexcept(std::is_nothrow_constructible<Tp, const Up&>::value) {
    if (other)
      this->emplace(*other);
  }

  // Converting copy constructor explicit.
  template <typename Up, POLLY_OPT_REQ_EXPLICIT_CONVERT_COPY_CTOR(Tp, Up)>
  explicit optional(const optional<Up>& other)
      noexcept(std::is_nothrow_constructible<Tp, const Up&>::value) {
    if (other)
      this->emplace(*other);
  }

  // Converting move constructor conditionallly explicit.
  // The function does not participate in the overload resolution unless the
  // following conditions are met:
  // 1. std::is_constructible<Tp, Up&&>::value is true.
  // 2. Tp is not constructible or convertible from any expression of type
  //    (possibly const) std::optional<Up>, i.e, the following 8 type traits
  //    are all false:
  //      1) std::is_constructible<Tp, std::optional<Up>&>::value
  //      2) std::is_constructible<Tp, std::optional<Up>& const>::value
  //      3) std::is_constructible<Tp, std::optional<Up>&&>::value
  //      4) std::is_constructible<Tp, std::optional<Up>&& const>::value
  //      5) std::is_convertible<std::optional<Up>&, Tp>::value
  //      6) std::is_convertible<std::optional<Up>& const, Tp>::value
  //      7) std::is_convertible<std::optional<Up>&&, Tp>::value
  //      8) std::is_convertible<std::optional<Up>&& const, Tp>::value
  // This constructor is explicit if and only if std::is_convertible<Up&&, Tp>
  // is false.

#define POLLY_OPT_REQ_IMPLICIT_CONVERT_MOVE_CTOR(Tp, Up)      \
  Requires<                                                   \
      negation<std::is_same<Tp, Up>>,                              \
      std::is_constructible<Tp, Up&&>,                        \
      std::is_convertible<Up&&, Tp>,                          \
      negation<optional_internal::converts_from_optional<Tp, Up>>  \
  > = true

#define POLLY_OPT_REQ_EXPLICIT_CONVERT_MOVE_CTOR(Tp, Up)      \
  Requires<                                                   \
      negation<std::is_same<Tp, Up>>,                              \
      std::is_constructible<Tp, Up&&>,                        \
      negation<std::is_convertible<Up&&, Tp>>,                     \
      negation<optional_internal::converts_from_optional<Tp, Up>>  \
  > = true

  // Converting move constructor.
  template <typename Up, POLLY_OPT_REQ_IMPLICIT_CONVERT_MOVE_CTOR(Tp, Up)>
  optional(optional<Up>&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value) {
    if (other)
      this->emplace(std::move(*other));
  }

  // Converting move constructor.
  template <typename Up, POLLY_OPT_REQ_EXPLICIT_CONVERT_MOVE_CTOR(Tp, Up)>
  explicit optional(optional<Up>&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value) {
    if (other)
      this->emplace(std::move(*other));
  }

  // Constructs an optional object from object of type Tp (where Tp = value_type)
  // with the expression `std::forward<Up>(value)`.
  // The function does not participate in the overload resolution unless the
  // following conditions are met:
  // 1. std::is_constructible<Tp, Up&&>::value is true
  // 2. std::decay<U>::type, std::remove_cvref<U>::type is neither std::in_place_t
  //    nor std::optional<Tp>
  // This constructor is explicit if and only if std::is_convertible<Up&&, Tp> is
  // false.

#define POLLY_OPT_REQ_IMPLICIT_VALUE_CTOR(Tp, Up)             \
  Requires<                                                   \
      not_self<Up>,                                            \
      not_in_place<Up>,                                         \
      std::is_constructible<Tp, Up&&>,                        \
      std::is_convertible<Up&&, Tp>                          \
  > = true

#define POLLY_OPT_REQ_EXPLICIT_VALUE_CTOR(Tp, Up)             \
  Requires<                                                   \
      not_self<Up>,                                            \
      not_in_place<Up>,                                         \
      std::is_constructible<Tp, Up&&>,                        \
      negation<std::is_convertible<Up&&, Tp>>                     \
  > = true

  // Value constructor.
  template <typename Up = Tp, POLLY_OPT_REQ_IMPLICIT_VALUE_CTOR(Tp, Up)>
  constexpr optional(Up&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value)
      : optional_internal::optional_base<Tp>(in_place, std::forward<Up>(other)) {}

  // Value constructor.
  template <typename Up = Tp, POLLY_OPT_REQ_EXPLICIT_VALUE_CTOR(Tp, Up)>
  constexpr explicit optional(Up&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value)
      : optional_internal::optional_base<Tp>(in_place, std::forward<Up>(other)) {}

  // Assignment from nullopt
  optional& operator=(nullopt_t) noexcept {
    this->reset();
    return *this;
  }

  // Copy assignment operator
  optional& operator=(const optional&) = default;

  // Move assignment operator
  optional& operator=(optional&&) = default;

  // Value assignment operators
  // Perfect-forwarded assignment: depending on whether optional contains a
  // value before the call, the contained value is either direct-initialized
  // from std::forward<Up>(other) or assigned from std::forward<Up>(other).
  // The function does not participate in overload resolution unless the
  // following conditions is met:
  //    1. std::remove_cvref<Up>::type is not std::optional<Tp>
  //    2. std::is_constructible<Tp, Up>::value is true
  //    3. std::is_assignable<T&, U>::value is true
  //    4. at least one of the following is true
  //        a. Tp is not a scalar type
  //        b. std::decay<Up>::type is not Tp
#define POLLY_OPT_REQ_VALUE_ASSIGNMENT(Tp, Up)                \
  Requires<                                                   \
      not_self<Up>,                                           \
      std::is_constructible<Tp, Up>,                          \
      std::is_assignable<Tp&, Up>,                            \
      negation<conjunction<                                                \
          std::is_scalar<Tp>,                                 \
          std::is_same<Tp, typename std::decay<Up>::type>     \
      >>                                                      \
  > = true

  template <typename Up = Tp, POLLY_OPT_REQ_VALUE_ASSIGNMENT(Tp, Up)>
  optional& operator=(Up&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value &&
               std::is_nothrow_assignable<Tp&,  Up>::value) {
    if (this->is_engaged()) {
      this->get() = std::forward<Up>(other);
    } else {
      this->construct(std::forward<Up>(other));
    }
    return *this;
  }

  // Value copy/move assignment operators
  // if both optional and other do not contain a value, the function has no
  // effect.
  // if optional contains a value, but other does not, then the contained
  // value is destroyed by calling it's destructor. optional does not contain
  // a value after the call.
  // if other contains a value, then depending on whether optional contains
  // a value, the contained value is either direct initialized or assigned
  // from optional or std::move(*other). Note that a moved from optional still
  // contains a value.
  // These functions overloads do not participate  overload resolution unless
  // following conditions are met:
  //    1. T is not constructible, convertible or assignable from any expression
  //        of type (possibly const) std::optional<Up>, i.e, the following 12
  //        type traits are all false:
  //          + std::is_constructible<Tp, std::optional<Up>&>::value
  //          + std::is_constructible<Tp, std::optional<Up>& const>::value
  //          + std::is_constructible<Tp, std::optional<Up>&&>::value
  //          + std::is_constructible<Tp, std::optional<Up>&& const>::value
  //          + std::is_convertible<std::optional<Up>&, Tp>::value
  //          + std::is_convertible<std::optional<Up>& const, Tp>::value
  //          + std::is_convertible<std::optional<Up>&&, Tp>::value
  //          + std::is_convertible<std::optional<Up>&& const, Tp>::value
  //          + std::is_assignable<Tp&, std::optional<Up>&>::value
  //          + std::is_assignable<Tp&, std::optional<Up>& const>::value
  //          + std::is_assignable<Tp&, std::optional<Up>&&>::value
  //          + std::is_assignable<Tp&, std::optional<Up>&& const>::value
  //    2. For value copy assignment, std::is_constructible<Tp, const Up&> and
  //       std::is_assignable<T&, const Up&> are both true.
  //    2. For value move assignment, std::is_constructible<Tp, Up> and
  //       std::is_assignable<T&, Up> are both true.
#define POLLY_OPT_REQ_VALUE_COPY_ASSIGNMENT(Tp, Up)           \
  Requires<                                                   \
      negation<std::is_same<Tp, Up>>,                              \
      negation<optional_internal::converts_from_optional<Tp, Up>>, \
      negation<optional_internal::assigns_from_optional<Tp, Up>>,  \
      std::is_constructible<Tp, const Up&>,                   \
      std::is_assignable<Tp&, const Up&>                      \
  > = true

#define POLLY_OPT_REQ_VALUE_MOVE_ASSIGNMENT(Tp, Up)           \
  Requires<                                                   \
      negation<std::is_same<Tp, Up>>,                              \
      negation<optional_internal::converts_from_optional<Tp, Up>>, \
      negation<optional_internal::assigns_from_optional<Tp, Up>>,  \
      std::is_constructible<Tp, Up>,                          \
      std::is_assignable<Tp&, Up>                             \
  > = true

  // Value copy assignment operators
  template <typename Up, POLLY_OPT_REQ_VALUE_COPY_ASSIGNMENT(Tp, Up)>
  optional& operator=(const optional<Up>& other)
      noexcept(std::is_nothrow_constructible<Tp, const Up&>::value &&
              std::is_nothrow_assignable<Tp&,  const Up&>::value) {
    if (other) {
      if (this->is_engaged()) {
        this->get() = *other;
      } else {
        this->construct(*other);
      }
    } else {
      this->reset();
    }
    return *this;
  }

  // Value move assignment operators
  template <typename Up, POLLY_OPT_REQ_VALUE_MOVE_ASSIGNMENT(Tp, Up)>
  optional& operator=(optional<Up>&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value &&
              std::is_nothrow_assignable<Tp&,  Up>::value) {
    if (other) {
      if (this->is_engaged()) {
        this->get() = std::move(*other);
      } else {
        this->construct(std::move(*other));
      }
    } else {
      this->reset();
    }
    return *this;
  }

  // Observers

  // operator->()
  // Accesses the contained value, returns a pointer to the contained value.
  // This operator does not check whether the optional contains a value. The
  // behavior is undefined if optional does not contain a value, You can do
  // so manually by using has_value() or simply operator bool(). Alternatively,
  // if check access is needed, value() or value_or() may be used.
  constexpr const Tp* operator->() const noexcept {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::addressof(this->get());
  }

  Tp* operator->() noexcept {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::addressof(this->get());
  }

  // operator*()
  // Like as operator->() but returns a reference to the contained value.
  constexpr const Tp& operator*() const & {
    return POLLY_CONST_ASSERT(this->is_engaged()), this->get();
  }

  Tp& operator*() & {
    return POLLY_CONST_ASSERT(this->is_engaged()), this->get();
  }

  constexpr const Tp&& operator*() const && {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::move(this->get());
  }

  Tp&& operator*() && {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::move(this->get());
  }

  // Checks whether optional contains a value. return true if contains a value,
  // return false otherwise.
  constexpr explicit operator bool() const noexcept {
    return this->is_engaged();
  }

  constexpr bool has_value() const noexcept {
    return this->is_engaged();
  }

  // If optional contains a value, returns a reference to the contained value.
  // Otherwise, throws a std::bad_optional_access exception. bad_optional_access
  // has be thrown if optional does not contains a value.
  constexpr const Tp& value() const & {
    return this->is_engaged()
      ? this->get()
      : (ThrowBadOptionalAccess(), this->get());
  }

  Tp& value() & {
    return this->is_engaged()
      ? this->get()
      : (ThrowBadOptionalAccess(), this->get());
  }

  Tp&& value() && {
    return std::move(
      this->is_engaged()
        ? this->get()
        : (ThrowBadOptionalAccess(), this->get())
    );
  }

  constexpr const Tp&& value() const && {
    return std::move(
      this->is_engaged()
        ? this->get()
        : (ThrowBadOptionalAccess(), this->get())
    );
  }

  // Return current value if optional has a value, or default value otherwise.
  // Tp must meet the requirements of CopyConstructible.
  // Up&& must be convertible to T.
  template <typename Up>
  constexpr const Tp value_or(Up&& default_value) const& {
    static_assert(std::is_copy_constructible<Tp>::value,
                  "optional<Tp>::value_or: Tp must be copy constructible");
    static_assert(std::is_convertible<Up&&, Tp>::value,
                  "optional<Tp>::value_or: Up must be convertible to Tp");
    return this->is_engaged()
        ? this->get()
        : static_cast<Tp>(std::forward<Up>(default_value));
  }

  // Return current value if optional has a value, or default value otherwise.
  // Tp must meet the requirements of MoveConstructible, Up&& must be convertible
  // to value_type.
  template <typename Up>
  Tp value_or(Up&& default_value) && {
    static_assert(std::is_move_constructible<Tp>::value,
                  "optional<Tp>::value_or: Tp must be copy constructible");
    static_assert(std::is_convertible<Up&&, Tp>::value,
                  "optional<Tp>::value_or: Up must be convertible to Tp");
    return this->is_engaged()
        ? std::move(this->get())
        : static_cast<Tp>(std::forward<Up>(default_value));
  }

  // Modifiers

  // Swap()
  // Swaps the contents with those of other. if neither optional and
  // nor other contains a value, the function has no effect. if only
  // one of optional and other contains a value, the contained value
  // of optional is direct initialized from other with move operator,
  // followed by destruction of the contained value of other. After
  // this call, other does not contain a value. if both optional and
  // other contain values, the contained value are exchanged by callling
  // `std::swap`, `Tp` lvalue must satisfy Swappable.
  void swap(optional& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               polly::is_nothrow_swappable<Tp>::value) {
    if (this->is_engaged() && other.is_engaged()) {
      std::swap(this->get(), other.get());
    } else if (this->is_engaged()) {
      other.construct(std::move(this->get()));
      this->reset();
    } else if (other.is_engaged()) {
      this->construct(std::move(other.get()));
      other.reset();
    }
  }

  // If optional contains a value, destroy that value as if by value().T::~T().
  // otherwise, there are no effects. optional does not contains a value after
  // this call.
  void reset() noexcept {
    optional_internal::optional_base<Tp>::reset();
  }

  // Constructs the contained value in place. if optional already contains
  // a value before the call, the contained value is destroyed by calling
  // its reset().
  template <typename... Args,
      Requires<std::is_constructible<Tp, Args...>> = true>
  Tp& emplace(Args&&... args) noexcept(
      std::is_nothrow_constructible<Tp, Args...>::value) {
    this->reset();
    this->construct(std::forward<Args>(args)...);
    return this->get();
  }

  template <typename Up, typename... Args,
      Requires<std::is_constructible<Tp, std::initializer_list<Up>&, Args...>> = true>
  Tp& emplace(std::initializer_list<Up> il, Args&&... args) noexcept(
      std::is_nothrow_constructible<Tp, std::initializer_list<Up>&, Args...>::value) {
    this->reset();
    this->construct(il, std::forward<Args>(args)...);
    return this->get();
  }
};

// Non-member functions

// Compare two optional objects
template <typename Tp, typename Up>
constexpr auto operator==(const optional<Tp>& lhs, const optional<Up>& rhs)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*lhs == *rhs) {
  return static_cast<bool>(lhs) == static_cast<bool>(rhs) &&
    (!lhs || *lhs == *rhs);
}

template <typename Tp, typename Up>
constexpr auto operator!=(const optional<Tp>& lhs, const optional<Up>& rhs)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*lhs != *rhs) {
  return static_cast<bool>(lhs) != static_cast<bool>(rhs) ||
      (static_cast<bool>(lhs) && *lhs != *rhs);
}

template <typename Tp, typename Up>
constexpr auto operator<(const optional<Tp>& lhs, const optional<Up>& rhs)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*lhs < *rhs) {
  return static_cast<bool>(rhs) && (!lhs || *lhs < *rhs);
}

template <typename Tp, typename Up>
constexpr auto operator<=(const optional<Tp>& lhs, const optional<Up>& rhs)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*lhs <= *rhs) {
  return !lhs || (static_cast<bool>(rhs) && *lhs <= *rhs);
}

template <typename Tp, typename Up>
constexpr auto operator>(const optional<Tp>& lhs, const optional<Up>& rhs)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*lhs > *rhs) {
  return static_cast<bool>(lhs) && (!rhs || *lhs > *rhs);
}

template <typename Tp, typename Up>
constexpr auto operator>=(const optional<Tp>& lhs, const optional<Up>& rhs)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*lhs >= *rhs) {
  return !rhs || (static_cast<bool>(lhs) && *lhs >= *rhs);
}

// Compare an optional object with a nullopt
template <typename Tp>
constexpr bool operator==(const optional<Tp>& opt, nullopt_t) noexcept {
  return !opt;
}

template <typename Tp>
constexpr bool operator==(nullopt_t, const optional<Tp>& opt) noexcept {
  return !opt;
}

template <typename Tp>
constexpr bool operator!=(const optional<Tp>& opt, nullopt_t) noexcept {
  return static_cast<bool>(opt);
}

template <typename Tp>
constexpr bool operator!=(nullopt_t, const optional<Tp>& opt) noexcept {
  return static_cast<bool>(opt);
}

template <typename Tp>
constexpr bool operator<(const optional<Tp>&, nullopt_t) noexcept {
  return false;
}

template <typename Tp>
constexpr bool operator<(nullopt_t, const optional<Tp>& opt) noexcept {
  return static_cast<bool>(opt);
}

template <typename Tp>
constexpr bool operator<=(const optional<Tp>& opt, nullopt_t) noexcept {
  return !opt;
}

template <typename Tp>
constexpr bool operator<=(nullopt_t, const optional<Tp>&) noexcept {
  return true;
}

template <typename Tp>
constexpr bool operator>(const optional<Tp>& opt, nullopt_t) noexcept {
  return static_cast<bool>(opt);
}

template <typename Tp>
constexpr bool operator>(nullopt_t, const optional<Tp>&) noexcept {
  return false;
}

template <typename Tp>
constexpr bool operator>=(const optional<Tp>&, nullopt_t) noexcept {
  return true;
}

template <typename Tp>
constexpr bool operator>=(nullopt_t, const optional<Tp>& opt) noexcept {
  return !opt;
}

// Compare an optional object with a object of type Tp
template <typename Tp, typename Up>
constexpr auto operator==(const optional<Tp>& opt, const Up& value)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt == value) {
  return opt && *opt == value;
}

template <typename Tp, typename Up>
constexpr auto operator==(const Up& value, const optional<Tp>& opt)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt == value) {
  return opt && value == *opt;
}

template <typename Tp, typename Up>
constexpr auto operator!=(const optional<Tp>& opt, const Up& value)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt != value) {
  return !opt || *opt != value;
}

template <typename Tp, typename Up>
constexpr auto operator!=(const Up& value, const optional<Tp>& opt)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt != value) {
  return !opt || value != *opt;
}

template <typename Tp, typename Up>
constexpr auto operator<(const optional<Tp>& opt, const Up& value)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt <= value) {
  return !opt || *opt < value;
}

template <typename Tp, typename Up>
constexpr auto operator<(const Up& value, const optional<Tp>& opt)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt < value) {
  return opt && value < *opt;
}

template <typename Tp, typename Up>
constexpr auto operator<=(const optional<Tp>& opt, const Up& value)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt <= value) {
  return !opt || *opt <= value;
}

template <typename Tp, typename Up>
constexpr auto operator<=(const Up& value, const optional<Tp>& opt)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt <= value) {
  return opt && value <= *opt;
}

template <typename Tp, typename Up>
constexpr auto operator>(const optional<Tp>& opt, const Up& value)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt > value) {
  return opt && *opt > value;
}

template <typename Tp, typename Up>
constexpr auto operator>(const Up& value, const optional<Tp>& opt)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt > value) {
  return !opt || value > *opt;
}

template <typename Tp, typename Up>
constexpr auto operator>=(const optional<Tp>& opt, const Up& value)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt >= value) {
  return opt && *opt >= value;
}

template <typename Tp, typename Up>
constexpr auto operator>=(const Up& value, const optional<Tp>& opt)
    -> POLLY_OPTIONAL_CONVERTIBLE_BOOL(*opt >= value) {
  return !opt || value >= *opt;
}

// Helper classes

template <typename Tp,
    Requires<std::is_constructible<typename std::decay<Tp>::type, Tp>> = true>
constexpr optional<typename std::decay<Tp>::type>
make_optional(Tp&& value) noexcept(
    std::is_nothrow_constructible<optional<typename std::decay<Tp>::type>, Tp>::value) {
  return optional<typename std::decay<Tp>::type>{std::forward<Tp>(value)};
}

template <typename Tp, typename... Args,
    Requires<std::is_constructible<Tp, Args...>> = true>
constexpr optional<Tp> make_optional(Args&&... args) noexcept(
    std::is_nothrow_constructible<Tp, Args...>::value) {
  return optional<Tp>{in_place, std::forward<Args>(args)...};
}

template <typename Tp,
    Requires<conjunction<std::is_move_constructible<Tp>, is_swappable<Tp>>> = true>
void swap(optional<Tp>& lhs, optional<Tp>& rhs)
    noexcept(noexcept(lhs.swap(rhs))) {
  lhs.swap(rhs);
}

} // namespace polly

namespace std {
template <typename Tp>
struct hash<polly::optional<Tp>>
    : public polly::optional_internal::optional_hash_base<Tp> {};
} // namespace std

#endif // POLLY_HAVE_STD_OPTIONAL