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

#include <typeinfo>

#if !defined(POLLY_HAVE_EXCEPTIONS)
#include "stubs/internal/raw_logging.h"
#endif

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

// Throw delegator
[[noreturn]] inline void ThrowBadAnyCast() {
#if !defined(POLLY_HAVE_EXCEPTIONS)
  POLLY_RAW_LOG(FATAL, bad_any_cast{}.what());
#else
  throw bad_any_cast{};
#endif
}

} // namespace polly

#endif // POLLY_HAVE_STD_ANY