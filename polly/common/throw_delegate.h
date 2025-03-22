// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_THROW_DELEGATE_H_
#define OAK_COMMON_THROW_DELEGATE_H_

#include <string>
#include "oak/common/macros.h"

namespace oak {

// std::logic_error
OAK_ATTR_NORETURN void ThrowStdLogicError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdLogicError(const char* what);

// std::domain_error
OAK_ATTR_NORETURN void ThrowStdDomainError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdDomainError(const char* what);

// std::invalid_argument
OAK_ATTR_NORETURN void ThrowStdInvalidArgument(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdInvalidArgument(const char* what);

// std::length_error
OAK_ATTR_NORETURN void ThrowStdLengthError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdLengthError(const char* what);

// std::out_of_range
OAK_ATTR_NORETURN void ThrowStdOutOfRange(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdOutOfRange(const char* what);

// std::runtime_error
OAK_ATTR_NORETURN void ThrowStdRuntimeError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdRuntimeError(const char* what);

// std::range_error
OAK_ATTR_NORETURN void ThrowStdRangeError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdRangeError(const char* what);

// std::overflow_error
OAK_ATTR_NORETURN void ThrowStdOverflowError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdOverflowError(const char* what);

// std::underflow_error
OAK_ATTR_NORETURN void ThrowStdUnderflowError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdUnderflowError(const char* what);

// std::system_error
OAK_ATTR_NORETURN void ThrowStdSystemError(const std::string& what);
OAK_ATTR_NORETURN void ThrowStdSystemError(const char* what);

}  // namespace oak

#endif  // OAK_COMMON_THROW_DELEGATE_H_
