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
  constexpr any() noexcept: empty_(true) {}

  // Copies or moves content of other into a new instance, so that any content
  // is equivalent in both type and value to those of other prior to the
  // constructor call, or empty if other is empty.
  any(const any& other) {

  }

  any(any&& other) noexcept;

  template <typename Tp>
  any(Tp&& value);

  template <typename Tp, typename... Args>
  explicit any(in_place_type_t<Tp>, Args&&... args);

  template <typename Tp, typename Up, typename... Args>
  explicit any(in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args);

  ~any();

  any& operator=(const any& rhs);

  any& operator=(any&& rhs) noexcept;

  template <typename Tp>
  any& operator=(Tp&& rhs);

  // Modifiers
  template <typename Tp, typename... Args>
  typename std::decay<Tp>::type emplace(Args&&... args);

  template <typename Tp, typename Up, typename... Args>
  typename std::decay<Tp>::type emplace(std::initializer_list<Up> il, Args&&... args);

  void reset() noexcept {
    if (has_value()) {

    }
  }

  void swap(any& other) noexcept;

  // Observers
  bool has_value() const noexcept {
    return empty_;
  }

#ifdef POLLY_HAVE_RTTI
  const std::type_info& type() const noexcept;
#endif

private:
  template <typename Tp>
  using normalized_type = remove_cvref_t<Tp>;






  // contained object
  template <typename Tp, bool = is_small_obj<Tp>::value>
  struct AnyOperator {
  };

  // in-place contained object
  template <typename Tp>
  struct AnyOperator<Tp, true> {
  };
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
