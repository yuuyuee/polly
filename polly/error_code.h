// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_ERROR_CODE_H_
#define OAK_COMMON_ERROR_CODE_H_

#include <errno.h>

namespace oak {

#define OAK_MAKE_ERROR(a, b, c, d) \
  (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define OAK_ERROR_HOLD OAK_MAKE_ERROR('H', 'O', 'L', 'D')

#define OAK_ERROR_MAP(XX) \
  XX(OAK_ERROR_HOLD, "Hold")

const char* Strerror(int errnum);
}  // namespace oak

#endif  // OAK_COMMON_ERROR_CODE_H_
