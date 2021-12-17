#pragma once

#include "stubs/internal/config.h"
#include <cassert>

#define POLLY_ARRAY_SIZE(a) sizeof(::polly::macros_internal::ArraySizeHelper(a))

# define POLLY_CONST_ASSERT(exp)  \
    (POLLY_EXPECT_TRUE(exp) ? static_cast<void>(0) : [] { assert(false && #exp); }())

// inline variables enabled in C++17.
#if defined(POLLY_HAVE_INLINE_VARIABLES)
# define POLLY_INLINE_CONSTEXPR(type_, name, init) \
  inline constexpr ::polly::macros_internal::identity_t<type_> name = init
#else // __cpp_inline_variables
# define POLLY_INLINE_CONSTEXPR(type_, name, init) \
  static constexpr ::polly::macros_internal::identity_t<type_> name{init}
#endif // __cpp_inline_variables

namespace polly {
namespace macros_internal {

template<typename Tp, size_t N>
auto ArraySizeHelper(const Tp (&a)[N]) -> char (&)[N];

template<typename Tp>
struct identity {
  using type = Tp;
};

template<typename Tp>
using identity_t = typename identity<Tp>::type;

} // namespace macros_internal
} // namespace polly