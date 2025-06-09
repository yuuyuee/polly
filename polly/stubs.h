// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_COMMON_STUBS_H_
#define POLLY_COMMON_STUBS_H_

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
#       define POLLY_LITTLE_ENDIAN 1
#   else
#       define POLLY_BIG_ENDIAN 1
#   endif
#else
#   error "Unable to detect the byte-order."
#endif

// __has_builtin check
#if defined(__has_builtin)
#   define POLLY_HAS_BUILTIN(x) __has_builtin(x)
#else
#   define POLLY_HAS_BUILTIN(x) 0
#endif

// __has_attribute check
#if defined(__has_attribute)
#   define POLLY_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#   define POLLY_HAS_ATTRIBUTE(x) 0
#endif

// __has_cpp_attribute check
#if defined(__has_cpp_attribute)
#   define POLLY_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#   define POLLY_HAS_CPP_ATTRIBUTE(x) 0
#endif

// __has_include check
#if defined(__has_include)
#   define POLLY_HAS_INCLUDE(x) __has_include(x)
#else
#   define POLLY_HAS_INCLUDE(x) 0
#endif

// Auxiliary macros

#define POLLY_DO_TOKEN_STR(x) #x
#define POLLY_TOKEN_STR(x) POLLY_DO_TOKEN_STR(x)

namespace rctems {
namespace stubs_internal {
template <typename T, size_t N>
auto ArraySizeHelper(const T (&)[N]) -> char (&)[N];
}  // namespace stubs_internal
}  // namespace rctems

#define POLLY_ARRAYSIZE(x)     \
    (sizeof(::rctems::stubs_internal::ArraySizeHelper(x)))

#if POLLY_HAS_BUILTIN(__builtin_expect)
#   define POLLY_LIKELY(x) __builtin_expect((x), true)
#   define POLLY_UNLIKELY(x) __builtin_expect((x), false)
#else
#   define POLLY_LIKELY(x) (x)
#   define POLLY_UNLIKELY(x) (x)
#endif

// Assert function that can be used within constexpr function.
#ifdef NDEBUG
#   define POLLY_ASSERT(expr) static_cast<void>(0)
#else
#   include <cassert>
#   define POLLY_ASSERT(expr)                      \
    (POLLY_LIKELY((expr)) ? static_cast<void>(0)   \
        : [] { assert(false && POLLY_DO_TOKEN_STR(expr)); }())
#endif

// Cache line
#if defined(__GNUC__)
#   if defined(__x86_64__)
#       define POLLY_CACHELINE_SIZE 64
#   elif defined(__aarch64__)
#       define POLLY_CACHELINE_SIZE 64
#   elif defined(__arm__)
#       if defined(__ARM_ARCH_5T__)
#           define POLLY_CACHELINE_SIZE 32
#       elif defined(__ARM_ARCH_7A__)
#           define POLLY_CACHELINE_SIZE 64
#       endif
#   endif
#else
#   define POLLY_CACHELINE_SIZE 64
#endif

// #if POLLY_HAS_ATTRIBUTE(aligned)
// #   define POLLY_CACHELINE_ALIGNED __attribute__((aligned(POLLY_CACHELINE_SIZE)))
// #else
// #   define POLLY_CACHELINE_ALIGNED
// #endif
#ifdef POLLY_CACHELINE_SIZE
#   define POLLY_CACHELINE_ALIGNED alignas(POLLY_CACHELINE_SIZE)
#else
#   define POLLY_CACHELINE_ALIGNED
#endif

#define POLLY_DISALLOW_COPY_AND_ASSIGN(type)   \
    type(type const&) = delete;                 \
    void operator=(type const&) = delete

#define POLLY_DISALLOW_MOVE_COPY_AND_MOVE_ASSIGN(type) \
    type(type&&) = delete;                              \
    void operator=(type&&) = delete

#endif  // POLLY_COMMON_STUBS_H_
