// Copyright 2022 The Oak Authors.

#include "oak/common/throw_delegate.h"

#include <errno.h>
#include <stdexcept>
#include <system_error>  // NOLINT

#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
# define OAK_THROW(exc) throw exc
#else
# include <stdlib.h>  // for abort
# define OAK_THROW(exc) abort()
#endif

namespace oak {

// std::logic_error
void ThrowStdLogicError(const std::string& what) {
  OAK_THROW(std::logic_error{what.c_str()});
}

void ThrowStdLogicError(const char* what) {
  OAK_THROW(std::logic_error{what});
}

// std::domain_error
void ThrowStdDomainError(const std::string& what) {
  OAK_THROW(std::domain_error{what.c_str()});
}

void ThrowStdDomainError(const char* what) {
  OAK_THROW(std::domain_error{what});
}

// std::invalid_argument
void ThrowStdInvalidArgument(const std::string& what) {
  OAK_THROW(std::invalid_argument{what.c_str()});
}

void ThrowStdInvalidArgument(const char* what) {
  OAK_THROW(std::invalid_argument{what});
}

// std::length_error
void ThrowStdLengthError(const std::string& what) {
  OAK_THROW(std::length_error{what.c_str()});
}

void ThrowStdLengthError(const char* what) {
  OAK_THROW(std::length_error{what});
}

// std::out_of_range
void ThrowStdOutOfRange(const std::string& what) {
  OAK_THROW(std::out_of_range{what.c_str()});
}

void ThrowStdOutOfRange(const char* what) {
  OAK_THROW(std::out_of_range{what});
}

// std::runtime_error
void ThrowStdRuntimeError(const std::string& what) {
  OAK_THROW(std::runtime_error{what.c_str()});
}

void ThrowStdRuntimeError(const char* what) {
  OAK_THROW(std::runtime_error{what});
}

// std::range_error
void ThrowStdRangeError(const std::string& what) {
  OAK_THROW(std::range_error{what.c_str()});
}

void ThrowStdRangeError(const char* what) {
  OAK_THROW(std::range_error{what});
}

// std::overflow_error
void ThrowStdOverflowError(const std::string& what) {
  OAK_THROW(std::overflow_error{what.c_str()});
}

void ThrowStdOverflowError(const char* what) {
  OAK_THROW(std::overflow_error{what});
}

// std::underflow_error
void ThrowStdUnderflowError(const std::string& what) {
  OAK_THROW(std::underflow_error{what.c_str()});
}

void ThrowStdUnderflowError(const char* what) {
  OAK_THROW(std::underflow_error{what});
}

// std::system_error
OAK_ATTR_NORETURN void ThrowStdSystemError(const std::string& what) {
  OAK_THROW(std::system_error(errno, std::system_category(), what.c_str()));
}

OAK_ATTR_NORETURN void ThrowStdSystemError(const char* what) {
  OAK_THROW(std::system_error(errno, std::system_category(), what));
}

}  // namespace oak
