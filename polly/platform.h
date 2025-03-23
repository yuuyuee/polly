/* Copyright 2022 The Oak Authors. */

#ifndef OAK_ADDONS_PUBLIC_PLATFORM_H_
#define OAK_ADDONS_PUBLIC_PLATFORM_H_

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__linux__) && !defined(linux) && !defined(__linux)
# error "This library requires linux operator system."
#endif

#if !defined(__x86_64__) && !defined(__x86_64) && !defined(_M_X64)
# error "This library requires x86 architecture with 64-bit extensions."
#endif

#ifdef __cplusplus
}
#endif

#endif /* OAK_ADDONS_PUBLIC_PLATFORM_H_ */
