#pragma once

#include "stubs/base/config.h"
#include <cassert>

#if defined(NDEBUG)
# define POLLY_CONST_ASSERT(exp) \
    (false ? static_cast<void>(exp) : static_cast<void>(0))
#else
# define POLLY_CONST_ASSERT(expr) \
    (POLLY_EXPECT_TRUE(expr) ? static_cast<void>(0) : [] { assert(false && #exp); }())
#endif

namespace polly {
namespace macros_internal {
template<typename T, size_t N, typename R = char (&)[N]>
R ArraySizeHelper(const T (&a)[N]);
} // namespace macros_internal
} // namespace polly

#define POLLY_ARRAY_SIZE(a) sizeof(::polly::macros_internal::ArraySizeHelper(a))