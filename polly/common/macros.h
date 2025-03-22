// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_MACROS_H_
#define OAK_COMMON_MACROS_H_

#define OAK_ARRAYSIZE(a) (sizeof(oak::macros_internal::ArraySizeHelper(a)))

#include <stddef.h>  // size_t

namespace oak {
namespace macros_internal {
template <typename T, size_t N, typename R = char (&)[N]>
R ArraySizeHelper(const T (&)[N]);
}  // namespace macros_internal
}  // namespace oak

#include "oak/addons/public/compiler.h"

// OAK_HAS_ATTRIBUTE()
#if defined(__has_attribute)
# define OAK_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
# define OAK_HAS_ATTRIBUTE(x) (0)
#endif

// OAK_HAS_CPP_ATTRIBUTE()
#if defined(__has_cpp_attribute)
# define OAK_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
# define OAK_HAS_CPP_ATTRIBUTE(x) (0)
#endif

// OAK_ATTR_ALWAYS_INLINE()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(always_inline)
# define OAK_ATTR_ALWAYS_INLINE inline __attribute__((always_inline))
#else
# define OAK_ATTR_ALWAYS_INLINE
#endif

// OAK_ATTR_NORETURN()
#if defined(__cplusplus) && __cplusplus >= 201103L
# define OAK_ATTR_NORETURN [[noreturn]]
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(noreturn)
# define OAK_ATTR_NORETURN __attribute__((noreturn))
#else
# define OAK_ATTR_NORETURN
#endif

// OAK_ATTR_DEPRECATED()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(deprecated)
# define OAK_ATTR_DEPRECATED(...) __attribute__((deprecated(__VA_ARGS__)))
#else
# define OAK_ATTR_DEPRECATED(...)
#endif

// OAK_ATTR_MAYBE_UNUSED()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(unused)
# define OAK_ATTR_MAYBE_UNUSED __attribute__((unused))
#else
# define OAK_ATTR_MAYBE_UNUSED
#endif

// OAK_ATTR_NODISCARD()
#if defined(__cplusplus) && __cplusplus >= 201103L
# define OAK_ATTR_NODISCARD [[nodiscard]]
#elif (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(warn_unused_result)
# define OAK_ATTR_NODISCARD __attribute__((warn_unused_result))
#else
# define OAK_ATTR_NODISCARD
#endif

// OAK_ATTR_ALIGNED()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(aligned)
# define OAK_ATTR_ALIGNED(n) __attribute__((aligned(n)))
#else
# define OAK_ATTR_ALIGNED(n)
#endif

// OAK_ATTR_PACKED()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(packed)
# define OAK_ATTR_PACKED __attribute__((packed))
#else
# define OAK_ATTR_PACKED
#endif

// OAK_ATTR_PRINTF()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_ATTRIBUTE(__format__)
# define OAK_ATTR_PRINTF(index, first) \
    __attribute__((__format__(__printf__, index, first)))
#else
# define OAK_ATTR_PRINTF(index, first)
#endif

// OAK_HAS_BUILTIN()
#if defined(__has_builtin)
# define OAK_HAS_BUILTIN(x) __has_builtin(x)
#else
# define OAK_HAS_BUILTIN(x) 0
#endif

// OAK_LIKELY(), OAK_UNLIKELY()
#if (defined(__GNUC__) && !defined(__clang__)) || \
    OAK_HAS_BUILTIN(__builtin_expect)
# define OAK_LIKELY(x) __builtin_expect(false || (x), true)
# define OAK_UNLIKELY(x) __builtin_expect(false || (x), false)
#else
# define OAK_LIKELY(x) (x)
# define OAK_UNLIKELY(x) (x)
#endif

#include "oak/addons/public/platform.h"

#ifndef OAK_CACHELINE_SIZE
# define OAK_CACHELINE_SIZE 64
#endif

#define OAK_CACHELINE_ALIGNED OAK_ATTR_ALIGNED(OAK_CACHELINE_SIZE)

// IGNORE_UNUNSED()
#define IGNORE_UNUESD(...) oak::macros_internal::IgnoreUnused(__VA_ARGS__)

namespace oak {
namespace macros_internal {
template <typename... Args>
constexpr int IgnoreUnused(Args const&...) { return 0; }

template <typename...>
constexpr int IgnoreUnused() { return 0; }
}  // namespace macros_internal
}  // namespace oak

// OAK_ASSERT()
// This is an runtime assert and that can be used portably within
// constexpr functions.
#if defined(NDEBUG)
# define OAK_ASSERT(cond)                                 \
    (false ? static_cast<void>(cond) : static_cast<void>(0))
#else
# include <assert.h>
# define OAK_ASSERT(cond)                       \
    (OAK_LIKELY(cond)                           \
        ? static_cast<void>(0)                  \
        : [] { assert(false && #cond); }())
#endif

#define OAK_DISALLOW_COPY_AND_ASSIGN(type)      \
    type(type const&) = delete;                 \
    type& operator=(type const&) = delete

#endif  // OAK_COMMON_MACROS_H_
