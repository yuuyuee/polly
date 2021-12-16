#pragma once

#include "stubs/internal/config.h"
#include "stubs/identity.h"

namespace polly {
namespace macros_internal {
template<typename T, size_t N>
auto ArraySizeHelper(const T (&a)[N]) -> char (&)[N];
} // namespace macros_internal
} // namespace polly

#define POLLY_ARRAY_SIZE(a) sizeof(::polly::macros_internal::ArraySizeHelper(a))

// inline variables enabled in C++17.
#if defined(POLLY_HAVE_INLINE_VARIABLES)
# define POLLY_INLINE_CONSTEXPR(type_, name, init) \
  inline constexpr ::polly::identity_t<type_> name = init
#else // __cpp_inline_variables
# define POLLY_INLINE_CONSTEXPR(type_, name, init) \
  static constexpr ::polly::identity_t<type_> name{init}
#endif // __cpp_inline_variables