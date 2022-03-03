#pragma once

#include "polly/stubs/config.h"

#if !defined(POLLY_HAVE_INLINE_VARIABLES)
#include "polly/stubs/type_traits.h"

// Emulation of C++17 inline variables based on whether or not the feature
// isi supported.
//
// Example:
//    Equivalent to `inline constexpr size_t variant_npos = -1;`
//    POLLY_INLINE_CONSTEXPR(size_t, variant_npos, -1);
//
// Note:
//    For language-level inline variable, decltype(name) will  be the type
//    that was specified along with const qualification. For emulated inline
//    variables, decltype(name) may be different (in parctive it will likely
//    be a reference type).

# define POLLY_INLINE_CONSTEXPR(type_, name, init)                      \
  template <typename = void>                                            \
  struct PollyInternalInlineVariableHolder_ ## name {                   \
    static constexpr ::polly::type_identity_t<type_>                    \
        kInstance = init;                                               \
  };                                                                    \
  template <typename Tp>                                                \
  constexpr ::polly::macros_internal::type_identity_t<type_>            \
      PollyInternalInlineVariableHolder_ ## name<Tp>::kInstance;        \
  static constexpr const ::polly::type_identity_t<type_>&               \
      name = PollyInternalInlineVariableHolder_ ## name<>::kInstance;   \
  static_assert(sizeof(void(*)(decltype(name))) != 0,                   \
                "Silence unused variable warnings.");

#else // !defined(POLLY_HAVE_INLINE_VARIABLES)

#define POLLY_INLINE_CONSTEXPR(type_, name, init)                       \
    inline constexpr ::polly::type_identity_t<type_> name = init

#endif // !defined(POLLY_HAVE_INLINE_VARIABLES)