#pragma once

#include "stubs/base/config.h"

namespace polly {
namespace macros_internal {
template<typename T, size_t N, typename R = char (&)[N]>
R ArraySizeHelper(const T (&a)[N]);
} // namespace macros_internal
} // namespace polly

#define POLLY_ARRAY_SIZE(a) sizeof(::polly::macros_internal::ArraySizeHelper(a))