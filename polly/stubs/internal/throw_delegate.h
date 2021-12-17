#pragma once

#include "stubs/internal/config.h"
#include <string>

namespace polly {
namespace throw_delegate_internal {

// std::logic_error
[[noreturn]] void ThrowStdLogicError(const std::string& what);
[[noreturn]] void ThrowStdLogicError(const char* what);

// std::domain_error
[[noreturn]] void ThrowStdDomainError(const std::string& what);
[[noreturn]] void ThrowStdDomainError(const char* what);

// std::invalid_argument
[[noreturn]] void ThrowStdInvalidArgument(const std::string& what);
[[noreturn]] void ThrowStdInvalidArgument(const char* what);

// std::length_error
[[noreturn]] void ThrowStdLengthError(const std::string& what);
[[noreturn]] void ThrowStdLengthError(const char* what);

// std::out_of_range
[[noreturn]] void ThrowStdOutOfRange(const std::string& what);
[[noreturn]] void ThrowStdOutOfRange(const char* what);

// std::runtime_error
[[noreturn]] void ThrowStdRuntimeError(const std::string& what);
[[noreturn]] void ThrowStdRuntimeError(const char* what);

// std::range_error
[[noreturn]] void ThrowStdRangeError(const std::string& what);
[[noreturn]] void ThrowStdRangeError(const char* what);

// std::overflow_error
[[noreturn]] void ThrowStdOverflowError(const std::string& what);
[[noreturn]] void ThrowStdOverflowError(const char* what);

// std::underflow_error
[[noreturn]] void ThrowStdUnderflowError(const std::string& what);
[[noreturn]] void ThrowStdUnderflowError(const char* what);

} // namespace throw_delegate_internal
} // namespace polly