// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_FORMAT_H_
#define OAK_COMMON_FORMAT_H_

#include <stdarg.h>
#include <string>
#include "oak/common/macros.h"

namespace oak {

// This is an simple wrapper for snprintf and vsnprintf.
size_t format(char* buffer, size_t size, const char* fmt, ...)
    OAK_ATTR_PRINTF(3, 4);
size_t format(char* buffer, size_t size, const char* fmt, va_list ap);

// Formatted output to string.
std::string Format(const char* fmt, ...) OAK_ATTR_PRINTF(1, 2);
std::string Format(const char* fmt, va_list ap);
}  // namespace oak

#endif  // OAK_COMMON_FORMAT_H_
