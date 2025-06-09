// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_COMMON_LOGGING_H_
#define POLLY_COMMON_LOGGING_H_

#include <string>

#include "boost/log/trivial.hpp"

namespace polly {

namespace logging_internal {
// Strip directory which any leading directory compoents has removed.
constexpr const char* Basename(const char* fname, int off) {
  return off == 0 || fname[off - 1] == '/'
      ? fname + off : Basename(fname, off - 1);
}

#define RCTEMS_FILE     \
    ::polly::logging_internal::Basename(__FILE__, sizeof(__FILE__) - 1)

}  // namespace logging_internal

#define RCTEMS_LOG(sev)     \
    BOOST_LOG_TRIVIAL(sev) << RCTEMS_FILE << ":" << __LINE__ << " "

#define RCTEMS_TRACE()    RCTEMS_LOG(trace)
#define RCTEMS_DEBUG()    RCTEMS_LOG(debug)
#define RCTEMS_INFO()     RCTEMS_LOG(info)
#define RCTEMS_WARNING()  RCTEMS_LOG(warning)
#define RCTEMS_ERROR()    RCTEMS_LOG(error)
#define RCTEMS_FATAL()    RCTEMS_LOG(fatal)

void InitLogSystem(const std::string& severity, const std::string& sinks);
}  // namespace polly

#endif  // POLLY_COMMON_LOGGING_H_
