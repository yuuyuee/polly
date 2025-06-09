// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

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
