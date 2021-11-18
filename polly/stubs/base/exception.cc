#include "stubs/base/exception.h"
#include <stdexcept>
#include <utility>
#include "stubs/base/check.h"

namespace polly {
namespace exception_internal {
template<typename Exception>
POLLY_ATTR_NORETURN void ThrowDelegate(const Exception& ex) {
#if defined(POLLY_HAVE_EXCEPTIONS)
  throw ex;
#else
  POLLY_SAFE_WRITE(ex.what());
#endif
}
} // namespace exception_internal

#define POLLY_MAKE_EXCEPTIONS_FN(XX)                  \
  XX(ThrowStdLogicError, std::logic_error)            \
  XX(ThrowStdDomainError, std::domain_error)          \
  XX(ThrowStdInvalidArgument, std::invalid_argument)  \
  XX(ThrowStdLengthError, std::length_error)          \
  XX(ThrowStdOutOfRange, std::out_of_range)           \
  XX(ThrowStdRuntimeError, std::runtime_error)        \
  XX(ThrowStdRangeError, std::range_error)            \
  XX(ThrowStdOverflowError, std::overflow_error)      \
  XX(ThrowStdUnderflowError, std::underflow_error)

#define POLLY_EXCEPTIONS_MAP(fn, type) \
  void fn(const char* what) { exception_internal::ThrowDelegate(type{what}); }  \
  void fn(const char* what) { exception_internal::ThrowDelegate(type{what}); }

POLLY_MAKE_EXCEPTIONS_FN(POLLY_EXCEPTIONS_MAP)

#undef POLLY_EXCEPTIONS_MAP
} // namespace polly