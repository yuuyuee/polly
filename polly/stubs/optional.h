#pragma once

#include "stubs/config.h"

#if defined(POLLY_HAVE_STD_OPTIONAL)
#include <optional>

namespace polly {
using std::bad_optional_access;
using std::optional;
using std::make_optional;
using std::nullopt_t;
using std::nullopt;
} // namespace polly

#else // POLLY_HAVE_STD_OPTIONAL

#include <initializer_list>
#include <type_traits>

#include "stubs/attributes.h"
#include "stubs/macros.h"
#include "stubs/utility.h"
#include "stubs/exception.h"
#include "stubs/enable_special_members.h"
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

// throw delegator
POLLY_ATTR_NORETURN inline
void ThrowBadOptionalAccess() {
  POLLY_THROW_OR_ABORT(bad_optional_access{});
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
template<typename Tp>
class optional
    : private optional_internal::optional_base<Tp>,
      private enable_copy_move<
        std::is_copy_structible<Tp>::value,   // Copy
        std::is_copy_structible<Tp>::value && // Copy assignment
        std::is_copy_assignable<Tp>::value,
        std::is_move_structible<Tp>::value,   // Move
        std::is_move_structible<Tp>::value && // Move assignment
        std::is_move_assignable<Tp>::value,
        optional<Tp>                          // Unique tag type
      > {
  static_assert(
      !std::is_same<nullopt_t, typename std::remove_cv<T>::type>::value,
      "optional<nullopt_t> is not allowed.");
  static_assert(
      !std::is_same<in_place_t, typename std::remove_cv<T>::type>::value,
      "optional<in_place_t> is not allowed.");
  static_assert(
      !std::is_reference<T>::value,
      "optional<reference> is not allowed.");
private:
  template<typename Up>
  using not_self =  Not<std::is_same<optional, remove_cvref_t<Up>>>;

  template<typename Up>
  using not_tag =  Not<std::is_same<in_place_t, remove_cvref_t<Up>>>
public:
  using value_type = Tp;

  // Constructs an optional object holding an empty value.
  constexpr optional() noexcept {}

  // Constructs an optional object with `nullopt` holding an empty value.
  constexpr optional(nullopt_t) noexcept {}

  // Constructs an optional object direct initialized with `args`.
  template<
    typename... Args,
    Requires<std::is_constructible<Tp, Args...>> = true
  >
  constexpr explicit optional(in_place_t, Args&&... args)
      noexcept(std::is_nothrow_constructible<Tp, Args...>::value)
      : optional_base<Tp>(in_place, std::forward<Args>(args)...) {}

  //
  template<
    typename Up,
    typename = Requires<
      Not<std::is_same<Tp, Up>>,
      std::is_constructible<Tp, const Up&>,
      std::is_convertible<const Up&, Tp>,
      Not<optional_internal::converts_from_optional<Tp, Up>>
    >
  >
  optional(const optional<Up>& other)
      noexcet(std::is_nothrow_constructible<Tp, const Up&>::value) {
    if (other)
      this->emplace(*other);
  }

  //
  template<
    typename Up,
    typename = Requires<
      Not<std::is_same<Tp, Up>>,
      std::is_constructible<Tp, const Up&>,
      Not<std::is_convertible<const Up&, Tp>>,
      Not<optional_internal::converts_from_optional<Tp, Up>>
    >
  >
  explicit optional(const optioanl<Up>& other)
      noexcept(std::is_nothrow_constructible<Tp, const Up&>::value) {
    if (other)
      this->emplace(*other);
  }

  //
  template<
    typename Up,
    typename = Requires<
      Not<std::is_same<Tp, Up>>,
      std::is_constructible<Tp, Up>,
      std::is_convertible<Up, Tp>,
      Not<optional_internal::converts_from_optional<Tp, Up>>
    >
  >
  optional(optional<Up>&& other)
      noexcet(std::is_nothrow_constructible<Tp, Up>::value) {
    if (other)
      this->emplace(std::move(*other));
  }

  //
  template<
    typename Up,
    typename = Requires<
      Not<std::is_same<Tp, Up>>,
      std::is_constructible<Tp, Up>,
      Not<std::is_convertible<Up, Tp>>,
      Not<optional_internal::converts_from_optional<Tp, Up>>
    >
  >
  optional(optioanl<Up>&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value) {
    if (other)
      this->emplace(std::move(*other));
  }

  // Value constructor implicit
  template<
    typename Up = Tp,
    typename = Requires<
      not_self<Up>,
      not_tag<Up>,
      std::is_constructible<Tp, Up>,
      std::is_convertiable<Up, Tp>
    >
  >
  constexpr optional(Up&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value)
      : optional_base<Tp>(in_place, std::forward<Up>(other)) {}

  // Value constructor explicit
  template<
    typename Up = Tp,
    typename = Requires<
      not_self<Up>,
      not_tag<Up>,
      std::is_constructible<Tp, Up>,
      Not<std::is_convertiable<Up, Tp>>
    >
  >
  constexpr explicit optional(Up&& other)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value)
      : optional_base<Tp>(in_place, std::forward<Up>(other)) {}

  // assignment
  optional& operator=(nullopt_t) noexcept { this->reset(); }

  // Perfect forwarded assignment, depending on whether optional contains
  // a value before the call, the contained value is either direct initialized
  // from `std::forward<Up>` or assigned from `std::forward<Up>`.
  template<typename Up = Tp>
  typename std::enable_if<
    not_self<Up>::value &&
    !(std::is_scalar<Tp>::value ||
      std::is_same<Tp, typename std::decay<Up>::type>::value) &&
    std::is_constructible<Tp, Up>::value &&
    std::is_assignable<Tp&, Up>::value,
    optional&
  >::type
operator=(Up&& other)
    noexcept(std::is_nothrow_constructible<Tp, Up>::value &&
             std::is_nothrow_assignable<Tp&,  Up>::value) {
  if (this->is_engaged()) {
    this->get() = std::forward<Up>(other);
  } else {
    this->construct(std::forward<Up>(other));
  }
  return *this;
}

  template<typename Up>
  typename std::enable_if<
    !(std::is_same<Tp, Up>::value ||
      std::is_constructible<Tp, const Up&>::value) &&
    std::is_assignable<Tp&, const Up&>::value &&
    !optional_internal::converts_from_optional<Tp, Up>::value &&
    !optional_internal::assigns_from_optional<Tp, Up>::value,
    optional&
  >::type
  operator=(const optional<Up>& other)
      noexcet(std::is_nothrow_constructible<Tp, const Up&>::value &&
              std::is_nothrow_assignable<Tp&,  const Up&>::value) {
    if (other) {
      if (this->engaged()) {
        this->get() = *other;
      } else {
        this->construct(*other);
      }
    } else {
      this->reset();
    }
  }

  template<typename Up>
  typename std::enable_if<
    !(std::is_same<Tp, Up>::value ||
      std::is_constructible<Tp, Up>::value) &&
    std::is_assignable<Tp&, Up>::value &&
    !optional_internal::converts_from_optional<Tp, Up>::value &&
    !optional_internal::assigns_from_optional<Tp, Up>::value,
    optional&
  >::type
  operator=(optional<Up>&& other)
      noexcet(std::is_nothrow_constructible<Tp, Up>::value &&
              std::is_nothrow_assignable<Tp&,  Up>::value) {
    if (other) {
      if (this->engaged()) {
        this->get() = std::move(*other);
      } else {
        this->construct(std::move(*other));
      }
    } else {
      this->reset();
    }
  }

  // Constructs the contained value in place. if optional already contains
  // a value before the call, the contained value is destroyed by calling
  // its reset().
  template<typename... Args>
  typename std::enable_if<
      std::is_constructible<value_type, Args...>, value_type&>::type
  emplace(Args&&...) noexcept(
      std::is_nothrow_constructible<value_type, Args...>::value) {
    this->reset();
    this->construct(std::forward<Args>(args)...);
    this->get();
  }

  // swap
  // Swaps the contents with those of other. if neither optional and
  // nor other contains a value, the function has no effect. if only
  // one of optional and other contains a value, the contained value
  // of optional is direct initialized from other with move operator,
  // followed by destruction of the contained value of other. After
  // this call, other does not contain a value. if both optional and
  // other contain values, the contained value are exchanged by callling
  // `std::swap`, `Tp` lvalue must satisfy Swappable.
  void swap(optional& other)
      noexcept(std::is_nothrow_move_constructible<value_type>::value &&
               std::is_nothrow_swappable<value_type>::value) {
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

  // observers

  // Accesses the contained value, returns a pointer or reference to
  // the contained value. This operator does not check whether the optional
  // contains avalue.
  constexpr const T* operator->() const noexcept {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::addressof(this->get());
  }

  constexpr T* operator->() noexcept {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::addressof(this->get());
  }

  constexpr const T& operator*() const& {
    return POLLY_CONST_ASSERT(this->is_engaged()), this->get();
  }

  constexpr T& operator*() & {
    return POLLY_CONST_ASSERT(this->is_engaged()), this->get();
  }

  constexpr T&& operator*() && {
    return POLLY_CONST_ASSERT(this->is_engaged()), std::move(this->get());
  }

  constexpr const T&& operator*() const&&;

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
  constexpr const value_type& value() const& {
    return this->is_engaged()
      ? this->get()
      : (ThrowBadOptionalAccess(), this->get());
  }

  constexpr value_type& value() & {
    return this->is_engaged()
      ? this->get()
      : (ThrowBadOptionalAccess(), this->get());
  }

  constexpr value_type&& value() && {
    return std::move(
      this->is_engaged()
        ? this->get()
        : (ThrowBadOptionalAccess(), this->get());
    );
  }

  // Return current value if optional has a value, or default value otherwise.
  // Tp must meet the requirements of CopyConstructible.
  // Up&& must be convertible to T.
  template<typename Up>
  constexpr value_type value_or(Up&& default_value) const& {
    static_assert(std::is_copy_constructible<value_type>::value);
    static_assert(std::is_convertiable<Up&&, value_type>::value);
    return this->is_engaged()
        ? this->get()
        : static_cast<value_type>(std::forward<Up>(default_value));
  }

  // Return current value if optional has a value, or default value otherwise.
  // value_type must meet the requirements of MoveConstructible.
  // Up&& must be convertible to value_type.
  template<typename Up>
  constexpr value_type value_or(Up&& default_value) && {
    static_assert(std::is_move_constructible<value_type>::value);
    static_assert(std::is_convertiable<Up&&, value_type>::value);
    return this->is_engaged()
        ? std::move(this->get())
        : static_cast<value_type>(std::forward<Up>(default_value));
  }

  // modifiers

  // If optional contains a value, destroy that value as if by value().T::~T().
  // otherwise, there are no effects. optional does not contains a value after
  // this call.
  using optional_base<Tp>::reset; // void reset() noexcept;
};
} // namespace polly

#endif // POLLY_HAVE_STD_OPTIONAL