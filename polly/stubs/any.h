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
#if defined(POLLY_HAVE_RTTI)
#include <typeinfo>
#endif

#if !defined(POLLY_HAVE_EXCEPTIONS)
#include "stubs/internal/raw_logging.h"
#endif
#include "stubs/internal/type_id.h"
#include "stubs/utility.h"

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
  constexpr any() noexcept;
  any(const any& other);
  any(any&& other) noexcept;

  template <typename Tp>
  any(Tp&& value);

  template <typename Tp, typename... Args>
  explicit any(in_place_type_t<Tp>, Args&&... args);

  template <typename Tp, typename Up, typename... Args>
  explicit any(in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args);

  ~any();

  // Modifier

private:

};

// Overloads the std::swap algorithm for any.
// Swaps the content of two any objects by calling lhs.swap(rhs)
void swap(any&, any&) noexcept;


} // namespace polly

#endif // POLLY_HAVE_STD_ANY
