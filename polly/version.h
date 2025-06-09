// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_VERSION_H_
#define RCTEMS_VERSION_H_

#include "rctems/common/stubs.h"

#define RCTEMS_VERSION_MAJOR 0
#define RCTEMS_VERSION_MINOR 1
#define RCTEMS_VERSION_PATCH 0

#define RCTEMS_VERSIOIN             \
    ((RCTEMS_VERSION_MAJOR << 16) | \
    (RCTEMS_VERSION_MINOR << 8) |   \
    (RCTEMS_VERSION_PATCH))

#define RCTEMS_VERSION_STR                      \
    RCTEMS_TOKEN_STR(RCTEMS_VERSION_MAJOR) "."  \
    RCTEMS_TOKEN_STR(RCTEMS_VERSION_MINOR) "."  \
    RCTEMS_TOKEN_STR(RCTEMS_VERSION_PATCH)

#endif  // RCTEMS_VERSION_H_
