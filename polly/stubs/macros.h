#pragma once

#include "stubs/config.h"

namespace polly {
namespace macros_internal {
template<typename T, size_t N>
auto ArraySizeHelper(const T (&a)[N]) -> char (&)[N];
} // namespace macros_internal
} // namespace polly

#define POLLY_ARRAY_SIZE(a) sizeof(::polly::macros_internal::ArraySizeHelper(a))