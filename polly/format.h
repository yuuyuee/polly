// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_COMMON_FORMAT_H_
#define POLLY_COMMON_FORMAT_H_

#if __cplusplus >= 202002L
#   include <format>

namespace polly {
using std::format;
using fmt::format_to;
using fmt::format_args;
}  // namespace rctems
#else
#   define FMT_HEADER_ONLY 1
#   include "fmt/format.h"

namespace polly {
using fmt::format;
using fmt::format_to;
using fmt::format_args;

template <typename... Args>
using format_string = fmt::format_string<Args...>;
}  // namespace polly
#endif

#endif  // POLLY_COMMON_FORMAT_H_
