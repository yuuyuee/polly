// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_COMMON_FORMAT_H_
#define RCTEMS_COMMON_FORMAT_H_

#if __cplusplus >= 202002L
#   include <format>

namespace rctems {
using std::format;
using fmt::format_to;
using fmt::format_args;
}  // namespace rctems
#else
#   define FMT_HEADER_ONLY 1
#   include "fmt/format.h"

namespace rctems {
using fmt::format;
using fmt::format_to;
using fmt::format_args;

template <typename... Args>
using format_string = fmt::format_string<Args...>;
}  // namespace rctems
#endif

#endif  // RCTEMS_COMMON_FORMAT_H_
