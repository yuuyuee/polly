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
  using base = optional_base<Tp>

  // constructor
  constexpr optional() noexcept {}

  constexpr optional(nullopt_t) noexcept {}


  optonal(const optionoal&) = default;
  optional(optional&&) = default;


  template<typename... Args>
  constexpr explicit optional(in_place_t, Args&&...)
      noexcept(std::is_nothrow_constructible<Tp, Args...>::value)
      : base(in_place, std::forward<Args>(args)...) {}

  // Converting constructor
  template<
    typename Up = Tp,
    typename = Requires<
      not_self<Up>,
      not_tag<Up>,
      std::is_constructible<Tp, Up>,
      std::is_convertiable<Up, Tp>
    >>
  constexpr optional(Up&& v)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value)
      : base(in_place, std::forward<Up>(v)) {}

  template<
    typename Up = Tp,
    typename = Requires<
      Not<std::is_same<optional, remove_cvref_t<Tp>>>,
      Not<std::is_same<in_place_t, remove_cvref_t<Tp>>>,
      std::is_constructible<Tp, Up>,
      Not<std::is_convertiable<Up, Tp>>
    >>
  constexpr optional(Up&& v)
      noexcept(std::is_nothrow_constructible<Tp, Up>::value)
      : base(in_place, std::forward<Up>(v)) {}

  // destructor
  ~optional() = default;

  // assignment
  optional& operator=(nullopt_t) noexcept;
  optional& operator=(const optional&);
  optional& operator=(optional&&) noexcept();
  template<typename U = T>
  optional& operator=(U&&);
  template<typename U>
  optional& operator=(const optional<U>&);
  template<typename U>
  optional& operator=(optional<U>&&);
  template<typename... Args>
  void emplace(Args&&...);
  template<typename U, typename... Args>
  void emplace(std::initializer_list<U>, Args&&...);

  // swap
  void swap(optional&) noexcept();

  // observers

  // Accesses the contained value, returns a pointer to the contained value.
  constexpr const T* operator->() const noexcept {
    return (POLLY_ASSERT(this->is_engaged()), std::addressof(this->get()));
  }

  constexpr T* operator->() noexcept {
    return (POLLY_ASSERT(this->is_engaged()), std::addressof(this->get()));
  }

  // Accesses the contained value, returns a reference to the contained value.
  constexpr const T& operator*() const& {
    return (POLLY_ASSERT(this->is_engaged()), this->get());
  }

  constexpr T& operator*() & {
    return (POLLY_ASSERT(this->is_engaged()), this->get());
  }

  constexpr T&& operator*() && {
    return (POLLY_ASSERT(this->is_engaged()), std::move(this->get()));
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
  using base::reset; // void reset() noexcept;
};
} // namespace polly

#endif // POLLY_HAVE_STD_OPTIONAL