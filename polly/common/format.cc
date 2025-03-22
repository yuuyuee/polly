// Copyright 2022 The Oak Authors.

#include <stdio.h>
#include "oak/common/format.h"

namespace oak {

// This is an simple wrapper for snprintf and vsnprintf.
size_t format(char* buffer, size_t size, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int ret = format(buffer, size, fmt, ap);
  if (ret < 0) {
    buffer[0] = '\0';
    ret = 0;
  }
  va_end(ap);
  return ret;
}

size_t format(char* buffer, size_t size, const char* fmt, va_list ap) {
  return vsnprintf(buffer, size, fmt, ap);
}

std::string Format(const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  std::string ret = Format(fmt, ap);
  va_end(ap);
  return ret;
}

std::string Format(const char* fmt, va_list ap) {
  char buffer[1024];
  int n = vsnprintf(buffer, sizeof(buffer), fmt, ap);
  return n > 0 ? std::string(buffer, n) : std::string();
}
}  // namespace oak
