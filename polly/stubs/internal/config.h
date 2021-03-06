// Copyright (c) 2022 The Polly Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0

// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#if !defined(__cplusplus)
# error "This library requires c++ compiler and support for the ISO C++ 2011 standard."
#endif

static_assert(__cplusplus >= 201103L,
    "This library must be enabled with the -std=c++11 compiler options.");

#if !defined(__GNUC__) || !defined(__GNUC_MINOR__)
# error "This library requires GNU compiler."
#endif

# ifdef __GNUC_PREREQ
#   define POLLY_GNUC_PREREQ(major, minor) __GNUC_PREREQ(major, minor)
# else
#   define POLLY_GNUC_PREREQ(major, minor) \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((major) << 16) + (minor))
# endif

static_assert(POLLY_GNUC_PREREQ(4, 8), "This library require GNUC version at least 4.8.x");

#include <features.h>

#if !defined(__GLIBC__) || !defined(__GLIBC_MINOR__)
# error "This library requires GNU C library."
#endif

# ifdef __GLIBC_PREREQ
#   define POLLY_GLIBC_PREREQ(major, minor) __GLIBC_PREREQ(major, minor)
# else
#   define POLLY_GLIBC_PREREQ(major, minor) \
  ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((major) << 16) + (minor))
# endif

#include <cstddef>

#if !defined(__GLIBCXX__)
# error "This library requires GNU C++ library."
#endif

#if !(defined(__linux__) || defined(linux) || defined(__linux))
# error "This library requires linux operator system."
#endif

#if !(defined(__LP64__) || defined(__LP64))
# error "This library requires operator system 64-bit."
#endif

// Checks whether the compiler both supports and enables exceptions.
#ifdef POLLY_HAVE_EXCEPTIONS
# error "POLLY_HAVE_EXCEPTIONS can not be directly set."
#elif defined(__cpp_exceptions) || defined(__EXCEPTIONS)
# define POLLY_HAVE_EXCEPTIONS 1
#endif

#ifdef POLLY_HAVE_RTTI
# error "POLLY_HAVE_RTTI can not be directly set."
#elif !defined(__GNUC__) || defined(__GXX_RTTI)
# define POLLY_HAVE_RTTI 1
#endif

// Checks the endianness of the platform.
#if defined(POLLY_IS_LITTLE_ENDIAN) || defined(POLLY_IS_BIG_ENDIAN)
# error "POLLY_IS_LITTLE_ENDIAN or POLLY_IS_BIG_ENDIAN can not be directly set."
#elif defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
# define POLLY_IS_LITTLE_ENDIAN 1
#else
# define POLLY_IS_BIG_ENDIAN 1
#endif

#if defined(__has_builtin)
# define POLLY_HAS_BUILTIN(...) __has_builtin(__VA_ARGS__)
#else
# define POLLY_HAS_BUILTIN(...) 0
#endif

// Available on GCC 5.0+
#if defined(__has_attribute)
#define POLLY_HAS_ATTRIBUTE(...) __has_attribute(__VA_ARGS__)
#else
#define POLLY_HAS_ATTRIBUTE(x) 0
#endif

#if defined(__has_cpp_attribute)
# define POLLY_HAS_CPP_ATTRIBUTE(...) __has_cpp_attribute(__VA_ARGS__)
#else
# define POLLY_HAS_CPP_ATTRIBUTE(...) 0
#endif

#if defined(__has_include)
# define POLLY_HAS_INCLUDE(...) __has_include(__VA_ARGS__)
#else
# define POLLY_HAS_INCLUDE(...) 0
#endif

#if POLLY_HAS_INCLUDE(<string_view>)
# define POLLY_HAVE_STD_STRING_VIEW 1
#endif

#if POLLY_HAS_INCLUDE(<optional>)
# define POLLY_HAVE_STD_OPTIONAL 1
#endif

#if POLLY_HAS_INCLUDE(<any>)
# define POLLY_HAVE_STD_ANY 1
#endif

#if POLLY_HAS_INCLUDE(<variant>)
# define POLLY_HAVE_STD_VARIANT 1
#endif

#if POLLY_HAS_INCLUDE(<span>)
# define POLLY_HAVE_STD_SPAN 1
#endif

// Available on GCC 4.8+
#if defined(__SANITIZE_ADDRESS__)
# define POLLY_HAVE_ADDRESS_SANITIZER 1
#endif

// Available on GCC 4.8+
#if defined(__SANITIZE_THREAD__)
# define POLLY_HAVE_THREAD_SANITIZER 1
#endif

// Available on GCC 4.9+
#if defined(__SANITIZE_MEMORY__)
# define POLLY_HAVE_MEMORY_SANITIZER 1
#endif

// C++ language features
#if __cpp_inline_variables >= 201606L
# define POLLY_HAVE_INLINE_VARIABLES 1
#endif

#if POLLY_GNUC_PREREQ(7, 4)
# define POLLY_HAVE_STD_IS_TRIVIALLY_DEFAULT_CONSTRUCTIBLE 1
# define POLLY_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE 1
# define POLLY_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE 1
#endif

# define POLLY_ATTR_ALWAYS_INLINE __attribute__((always_inline))
# define POLLY_ATTR_NOINLINE __attribute__((noinline))
# define POLLY_ATTR_DISABLE_TAIL_CALL \
    __attribute__((optimize("no-optimize-sibling-calls")))

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
#if POLLY_HAS_ATTRIBUTE(aligned) || defined(__GNUC__)
# define POLLY_CACHELINE_ALIGNED __attribute__((aligned(POLLY_CACHELINE_SIZE)))
#endif

#if POLLY_HAS_BUILTIN(__builtin_expect) || defined(__GNUC__)
# define POLLY_EXPECT_TRUE(x) __builtin_expect(false || (x), true)
# define POLLY_EXPECT_FALSE(x) __builtin_expect(false || (x), false)
#endif
