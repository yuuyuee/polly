#pragma once

#include "stubs/base/config.h"

#if POLLY_HAS_ATTRIBUTE(always_inline) || POLLY_GNUC_PREREQ(3, 1)
# define POLLY_ATTR_ALWAYS_INLINE __attribute__((always_inline))
#endif

#if POLLY_HAS_ATTRIBUTE(always_inline) || defined(__GNUC__)
# define POLLY_ATTR_NOINLINE __attribute__((noinline))
#endif

#if POLLY_HAS_ATTRIBUTE(disable_tail_calls) || defined(__GNUC__)
# define POLLY_ATTR_DISABLE_TAIL_CALL \
    __attribute__((optimize("no-optimize-sibling-calls")))
#endif

#if POLLY_HAS_CPP_ATTRIBUTE(noreturn)
# define POLLY_ATTR_NORETURN [[noreturn]]
#elif POLLY_HAS_ATTRIBUTE(noreturn) || defined(__GNUC__)
# define POLLY_ATTR_NORETURN __attribute__((noreturn))
#endif

#if POLLY_HAS_CPP_ATTRIBUTE(deprecated)
# define POLLY_ATTR_DEPRECATED(...) [[deprecated]]
#elif POLLY_HAS_ATTRIBUTE(deprecated) || defined(__GNUC__)
# define POLLY_ATTR_DEPRECATED(...) __attribute__((deprecated(__VA_ARGS__)))
#endif

#if POLLY_HAS_CPP_ATTRIBUTE(maybe_unused)
# define POLLY_ATTR_MAYBE_UNUSED [[maybe_unused]]
#elif POLLY_HAS_ATTRIBUTE(unused) || defined(__GNUC__)
# define POLLY_ATTR_MAYBE_UNUSED __attribute__((unused))
#endif

#if POLLY_HAS_CPP_ATTRIBUTE(nodiscard)
# define POLLY_ATTR_NODISCARD [[nodiscard]]
#elif POLLY_HAS_ATTRIBUTE(warn_unused_result) || defined(__GNUC__)
# define POLLY_ATTR_NODISCARD __attribute__((warn_unused_result))
#endif

#define POLLY_CACHELINE_SIZE 64
#define POLLY_CACHELINE_ALIGNED __attribute__((aligned(POLLY_CACHELINE_SIZE)))

#if POLLY_HAS_BUILTIN(__builtin_expect) || defined(__GNUC__)
#define POLLY_EXPECT_TRUE(x) __builtin_expect(false || (x), true)
#define POLLY_EXPECT_FALSE(x) __builtin_expect(false || (x), false)
#else
#define POLLY_EXPECT_TRUE(x) (x)
#define POLLY_EXPECT_FALSE(x) (x)
#endif