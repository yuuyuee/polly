// Copyright 2022 The Oak Authors.

#include "oak/common/error_code.h"

#include <string.h>
#include <stdio.h>

namespace oak {
namespace {
const char* OakStrerror(int errnum) {
  static const struct {
    int code;
    const char* str;
  } str_error[] = {
#define OAK_MAKE_ERROR_STR(c, s) {.code = c, .str = s},
    OAK_ERROR_MAP(OAK_MAKE_ERROR_STR)
#undef OAK_ERROR_HOLD
  };

  for (const auto& it : str_error) {
    if (it.code == errnum)
      return it.str;
  }
  return nullptr;
}
}  // anonymous namespace

const char* Strerror(int errnum) {
  constexpr int kBufferSize = 256;
  static thread_local char buffer[kBufferSize];

  const char* err = OakStrerror(errnum);
  if (err)
    return err;

  if (strerror_r(errnum, buffer, kBufferSize) != 0)
    snprintf(buffer, kBufferSize, "Unknown error");
  return buffer;
}

}  // namespace oak
