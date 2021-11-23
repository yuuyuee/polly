#pragma once

#include <string>
#include <cstdlib>
#include <stdexcept>

#include "stubs/attributes.h"

#if !defined(POLLY_HAVE_EXCEPTIONS)
#include "stubs/check.h"
#endif

namespace polly {
namespace exception_internal {
template<typename Exception>
POLLY_ATTR_NORETURN POLLY_ATTR_ALWAYS_INLINE
inline void ThrowDelegate(const Exception& ex) {
#if defined(POLLY_HAVE_EXCEPTIONS)
  throw ex;
#else
  POLLY_MESSAGE(ex.what());
  std::abort();
#endif
}
} // anonymous namespace

// Exception throw delegator

// Logic error
POLLY_ATTR_NORETURN void ThrowStdLogicError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdLogicError(const char* what);
POLLY_ATTR_NORETURN void ThrowStdDomainError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdDomainError(const char* what);
POLLY_ATTR_NORETURN void ThrowStdInvalidArgument(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdInvalidArgument(const char* what);
POLLY_ATTR_NORETURN void ThrowStdLengthError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdLengthError(const char* what);
POLLY_ATTR_NORETURN void ThrowStdOutOfRange(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdOutOfRange(const char* what);

// Runtime error
POLLY_ATTR_NORETURN void ThrowStdRuntimeError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdRuntimeError(const char* what);
POLLY_ATTR_NORETURN void ThrowStdRangeError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdRangeError(const char* what);
POLLY_ATTR_NORETURN void ThrowStdOverflowError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdOverflowError(const char* what);
POLLY_ATTR_NORETURN void ThrowStdUnderflowError(const std::string& what);
POLLY_ATTR_NORETURN void ThrowStdUnderflowError(const char* what);

} // namespace polly