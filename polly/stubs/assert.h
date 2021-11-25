#pragma once

#include <cstdlib>
#include <cassert>

#include "stubs/attributes.h"
#include "stubs/const.h"

namespace polly {
namespace assert_internal {
void SafeWriteToStderr(const char* file, int line, const char* fmt, ...);
} // namespace assert_internal
} // namespace polly

#define POLLY_MESSAGE(fmt, ...)  do {                                       \
  constexpr const char* basename = polly::ConstBasename(__FILE__);          \
  assert_internal::SafeWriteToStderr(basename, __LINE__, fmt, __VA_ARGS__);  \
} while (0)

#if defined(NDEBUG)
# define POLLY_CONST_ASSERT(exp)  \
    (false ? static_cast<void>(exp) : static_cast<void>(0))

#define POLLY_ASSERT(exp, msg)     \
    (false ? static_cast<void>(exp) : static_cast<void>(0))
#else
# define POLLY_CONST_ASSERT(exp)  \
    (POLLY_EXPECT_TRUE(exp) ? static_cast<void>(0) : [] { assert(false && #exp); }())

#define POLLY_ASSERT(exp, msg)                           \
  if (POLLY_EXPECT_FALSE(!(exp))) {                     \
    POLLY_MESSAGE("Check %s failed: %s", #exp, msg);    \
    std::abort();                                       \
  }
#endif