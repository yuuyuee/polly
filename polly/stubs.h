// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_COMMON_STUBS_H_
#define RCTEMS_COMMON_STUBS_H_

#include <limits.h>

// C++ versions check
#if !defined(__cplusplus) || __cplusplus < 201703L
#   error "C++ versions less than C++17 are not supported."
#endif
#include <cstddef>

// GCC check
#if defined(__GNUC__) && !defined(__clang__)
#   if __GNUC__ < 7
#       error "Requires GCC 7 or higher."
#   endif
#endif

// Operator system check
#if !defined(__linux__)
#   error "OS is not supported."
#endif

// Platform check
#if !defined(__x86_64__) && !defined(__arm__) && !defined(__aarch64__)
#   error "Platform is not supported."
#endif

// Byte order check
#if defined(__BYTE_ORDER__)
#   if defined(__ORDER_LITTLE_ENDIAN__)     \
        && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#       define RCTEMS_LITTLE_ENDIAN 1
#   else
#       define RCTEMS_BIG_ENDIAN 1
#   endif
#else
#   error "Unable to detect the byte-order."
#endif

// __has_builtin check
#if defined(__has_builtin)
#   define RCTEMS_HAS_BUILTIN(x) __has_builtin(x)
#else
#   define RCTEMS_HAS_BUILTIN(x) 0
#endif

// __has_attribute check
#if defined(__has_attribute)
#   define RCTEMS_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#   define RCTEMS_HAS_ATTRIBUTE(x) 0
#endif

// __has_cpp_attribute check
#if defined(__has_cpp_attribute)
#   define RCTEMS_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#   define RCTEMS_HAS_CPP_ATTRIBUTE(x) 0
#endif

// __has_include check
#if defined(__has_include)
#   define RCTEMS_HAS_INCLUDE(x) __has_include(x)
#else
#   define RCTEMS_HAS_INCLUDE(x) 0
#endif

// Auxiliary macros

#define RCTEMS_DO_TOKEN_STR(x) #x
#define RCTEMS_TOKEN_STR(x) RCTEMS_DO_TOKEN_STR(x)

namespace rctems {
namespace stubs_internal {
template <typename T, size_t N>
auto ArraySizeHelper(const T (&)[N]) -> char (&)[N];
}  // namespace stubs_internal
}  // namespace rctems

#define RCTEMS_ARRAYSIZE(x)     \
    (sizeof(::rctems::stubs_internal::ArraySizeHelper(x)))

#if RCTEMS_HAS_BUILTIN(__builtin_expect)
#   define RCTEMS_LIKELY(x) __builtin_expect((x), true)
#   define RCTEMS_UNLIKELY(x) __builtin_expect((x), false)
#else
#   define RCTEMS_LIKELY(x) (x)
#   define RCTEMS_UNLIKELY(x) (x)
#endif

// Assert function that can be used within constexpr function.
#ifdef NDEBUG
#   define RCTEMS_ASSERT(expr) static_cast<void>(0)
#else
#   include <cassert>
#   define RCTEMS_ASSERT(expr)                      \
    (RCTEMS_LIKELY((expr)) ? static_cast<void>(0)   \
        : [] { assert(false && RCTEMS_DO_TOKEN_STR(expr)); }())
#endif

// Cache line
#if defined(__GNUC__)
#   if defined(__x86_64__)
#       define RCTEMS_CACHELINE_SIZE 64
#   elif defined(__aarch64__)
#       define RCTEMS_CACHELINE_SIZE 64
#   elif defined(__arm__)
#       if defined(__ARM_ARCH_5T__)
#           define RCTEMS_CACHELINE_SIZE 32
#       elif defined(__ARM_ARCH_7A__)
#           define RCTEMS_CACHELINE_SIZE 64
#       endif
#   endif
#else
#   define RCTEMS_CACHELINE_SIZE 64
#endif

// #if RCTEMS_HAS_ATTRIBUTE(aligned)
// #   define RCTEMS_CACHELINE_ALIGNED __attribute__((aligned(RCTEMS_CACHELINE_SIZE)))
// #else
// #   define RCTEMS_CACHELINE_ALIGNED
// #endif
#ifdef RCTEMS_CACHELINE_SIZE
#   define RCTEMS_CACHELINE_ALIGNED alignas(RCTEMS_CACHELINE_SIZE)
#else
#   define RCTEMS_CACHELINE_ALIGNED
#endif

#define RCTEMS_DISALLOW_COPY_AND_ASSIGN(type)   \
    type(type const&) = delete;                 \
    void operator=(type const&) = delete

#define RCTEMS_DISALLOW_MOVE_COPY_AND_MOVE_ASSIGN(type) \
    type(type&&) = delete;                              \
    void operator=(type&&) = delete

#endif  // RCTEMS_COMMON_STUBS_H_
