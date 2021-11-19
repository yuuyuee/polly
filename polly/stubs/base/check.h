#pragma once

#include <cstdlib>

#include "stubs/base/attributes.h"
#include "stubs/base/const.h"

namespace polly {
namespace check_internal {
void SafeWriteToStderr(const char* file, int line, const char* fmt, ...);
} // namespace safe_write_internal
} // namespace polly

#define POLLY_SAFE_WRITE(fmt, ...)  do {                                    \
  constexpr const char* basename = polly::ConstBasename(__FILE__);          \
  check_internal::SafeWriteToStderr(basename, __LINE__, fmt, __VA_ARGS__);  \
} while (0)

#define POLLY_CHECK(exp, msg)                           \
  if (POLLY_EXPECT_FALSE(!(exp))) {                     \
    POLLY_SAFE_WRITE("Check %s failed: %s", #exp, msg); \
    std::abort();                                       \
  }