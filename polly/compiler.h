/* Copyright 2022 The Oak Authors. */

#ifndef OAK_ADDONS_PUBLIC_COMPILER_H_
#define OAK_ADDONS_PUBLIC_COMPILER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && !defined(__clang__)
# define OAK_GCC_PREREQ(major, minor) \
  ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((major) << 16) + (minor))

# if !OAK_GCC_PREREQ(4, 8)
#  error "This library require GCC version at least 4.8."
# endif
#elif defined(__clang__)
# define OAK_CLANG_PREREQ(major, minor) \
  ((__clang_major__ << 16) + __clang_minor__ >= ((major) << 16) + (minor))

# if !OAK_CLANG_PREREQ(3, 4)
#  error "This library require clang version at least 3.4."
# endif
#else  // !defined(__GNUC__) && !defined(__clang__)
# error "This library requires GCC or clang compiler."
#endif

#include <features.h>

#if defined(__GLIBC__)
# define OAK_GLIBC_PREREQ(major, minor) \
  ((__GLIBC__ << 16) + __GLIBC_MINOR__ >= ((major) << 16) + (minor))

# if !OAK_GLIBC_PREREQ(2, 17)
#  error "This library require glibc version at least 2.17."
# endif
#else
# error "This library requires glibc package."
#endif

#ifdef __cplusplus
}
#endif

#endif  /* OAK_ADDONS_PUBLIC_COMPILER_H_ */
