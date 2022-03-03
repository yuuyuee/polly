#pragma once

#include "polly/stubs/internal/config.h"
#include <cassert>
#include "polly/stubs/type_traits.h"

#define POLLY_ARRAY_SIZE(a) sizeof(::polly::macros_internal::ArraySizeHelper(a))

# define POLLY_CONST_ASSERT(exp)  \
    (POLLY_EXPECT_TRUE(exp) ? static_cast<void>(0) : [] { assert(false && #exp); }())

namespace polly {
namespace macros_internal {

template<typename Tp, size_t N>
auto ArraySizeHelper(const Tp (&a)[N]) -> char (&)[N];

} // namespace macros_internal
} // namespace polly