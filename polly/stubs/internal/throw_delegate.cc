#include "stubs/internal/throw_delegate.h"

#include <cstdlib>
#include <stdexcept>

#if !defined(POLLY_HAVE_EXCEPTIONS)
# include "stubs/internal/raw_logging.h"
#endif

namespace polly {
namespace throw_delegate_internal {
#if defined(POLLY_HAVE_EXCEPTIONS)
# define POLLY_THROW_OR_ABORT(ex) throw ex
#else // POLLY_HAVE_EXCEPTIONS
# define POLLY_THROW_OR_ABORT(ex) POLLY_RAW_LOG(FATAL, ex.what())
#endif // POLLY_HAVE_EXCEPTIONS

// std::logic_error
void ThrowStdLogicError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::logic_error{what});
}

void ThrowStdLogicError(const char* what) {
  POLLY_THROW_OR_ABORT(std::logic_error{what});
}

// std::domain_error
void ThrowStdDomainError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::domain_error{what});
}

void ThrowStdDomainError(const char* what) {
  POLLY_THROW_OR_ABORT(std::domain_error{what});
}

// std::invalid_argument
void ThrowStdInvalidArgument(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::invalid_argument{what});
}

void ThrowStdInvalidArgument(const char* what) {
  POLLY_THROW_OR_ABORT(std::invalid_argument{what});
}

// std::length_error
void ThrowStdLengthError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::length_error{what});
}

void ThrowStdLengthError(const char* what) {
  POLLY_THROW_OR_ABORT(std::length_error{what});
}

// std::out_of_range
void ThrowStdOutOfRange(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::out_of_range{what});
}

void ThrowStdOutOfRange(const char* what) {
  POLLY_THROW_OR_ABORT(std::out_of_range{what});
}

// std::runtime_error
void ThrowStdRuntimeError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::runtime_error{what});
}

void ThrowStdRuntimeError(const char* what) {
  POLLY_THROW_OR_ABORT(std::runtime_error{what});
}

// std::range_error
void ThrowStdRangeError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::range_error{what});
}

void ThrowStdRangeError(const char* what) {
  POLLY_THROW_OR_ABORT(std::range_error{what});
}

// std::overflow_error
void ThrowStdOverflowError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::overflow_error{what});
}

void ThrowStdOverflowError(const char* what) {
  POLLY_THROW_OR_ABORT(std::overflow_error{what});
}

// std::underflow_error
void ThrowStdUnderflowError(const std::string& what) {
  POLLY_THROW_OR_ABORT(std::underflow_error{what});
}

void ThrowStdUnderflowError(const char* what) {
  POLLY_THROW_OR_ABORT(std::underflow_error{what});
}

} // namespace throw_delegate_internal
} // namespace polly