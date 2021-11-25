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
template<typename Tp> class optional {
  static_assert(
      !std::is_same<nullopt_t, typename std::remove_cv<T>::type>::value,
      "optional<nullopt_t> is not allowed.");
  static_assert(
      !std::is_same<in_place_t, typename std::remove_cv<T>::type>::value,
      "optional<in_place_t> is not allowed.");
  static_assert(
      !std::is_reference<T>::value,
      "optional<reference> is not allowed.");
public:
  using value_type = T;

  // constructor
  constexpr optional() noexcept {}

  constexpr optional(nullopt_t) noexcept {}


  optonal(const optionoal&) = default;
  optional(optional&&) = default;


  template<typename... Args>
  constexpr explicit optional(in_place_t, Args&&...);

  template<typename U, typename... Args>
  constexpr explicit optional(in_place_t, std::initializer_list<U>, Args&&...);

  template<typename U = T>
  constexpr optional(U&&);

  template<typename U>
  optional(const optional<U>&);

  template<typename U>
  optional(optional<U>&&);

  // destructor
  ~optional() { Destruct(); }

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
  constexpr const T* operator->() const;
  constexpr T* operator->();
  constexpr const T& operator*() const&;
  constexpr T& operator*() &;
  constexpr T&& operator*() &&;
  constexpr const T&& operator*() const&&;
  constexpr explicit operator bool() const noexcept;
  constexpr bool has_value() const noexcept;
  constexpr const T& value() const&;
  constexpr T& value() &;
  constexpr T&& value() &&;
  template<typename U> constexpr T value_or(U&&) const&;
  template<typename U> constexpr T value_or(U&&) &&;

  // modifiers
  void reset() noexcept;

private:
  static_assert(sizeof(T) > 0, "optional::value_type incomplete type");

  void Desturct() {
    if (hash_value()) {
      data_.~T();
    }
  }

  bool has_value_;
  enum {
    T data_;
    char storage_[sizeof(T)];
  };
};
} // namespace polly

#endif // POLLY_HAVE_STD_OPTIONAL