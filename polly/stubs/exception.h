#pragma once

#include <cstdlib>
#include <string>
#include <stdexcept>

#include "stubs/attributes.h"
#if !defined(POLLY_HAVE_EXCEPTIONS)
#include "stubs/assert.h"
#endif

namespace polly {
#if defined(POLLY_HAVE_EXCEPTIONS)
# define POLLY_THROW_OR_ABORT(ex) throw ex
#else // POLLY_HAVE_EXCEPTIONS
# define POLLY_THROW_OR_ABORT(ex) do {
    POLLY_MESSAGE(ex.what());
    std::abort();
  } while (0)
#endif // POLLY_HAVE_EXCEPTIONS

#define POLLY_MAKE_EXCEPTIONS_FN(XX)                  \
  XX(ThrowStdLogicError,      std::logic_error)       \
  XX(ThrowStdDomainError,     std::domain_error)      \
  XX(ThrowStdInvalidArgument, std::invalid_argument)  \
  XX(ThrowStdLengthError,     std::length_error)      \
  XX(ThrowStdOutOfRange,      std::out_of_range)      \
  XX(ThrowStdRuntimeError,    std::runtime_error)     \
  XX(ThrowStdRangeError,      std::range_error)       \
  XX(ThrowStdOverflowError,   std::overflow_error)    \
  XX(ThrowStdUnderflowError,  std::underflow_error)

#define POLLY_EXCEPTIONS_MAP(fn, type)                \
  POLLY_ATTR_NORETURN inline                          \
  void fn(const char* what) {                         \
    POLLY_THROW_OR_ABORT(type{what});                 \
  }                                                   \
                                                      \
  POLLY_ATTR_NORETURN inline                          \
  void fn(const std::string& what) {                  \
    POLLY_THROW_OR_ABORT(type{what});                 \
  }

POLLY_MAKE_EXCEPTIONS_FN(POLLY_EXCEPTIONS_MAP)

#undef POLLY_EXCEPTIONS_MAP

} // namespace polly