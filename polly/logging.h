// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#ifndef RCTEMS_COMMON_LOGGING_H_
#define RCTEMS_COMMON_LOGGING_H_

#include <string>

#include "boost/log/trivial.hpp"

namespace rctems {

namespace logging_internal {
// Strip directory which any leading directory compoents has removed.
constexpr const char* Basename(const char* fname, int off) {
  return off == 0 || fname[off - 1] == '/'
      ? fname + off : Basename(fname, off - 1);
}

#define RCTEMS_FILE     \
    ::rctems::logging_internal::Basename(__FILE__, sizeof(__FILE__) - 1)

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
}  // namespace rctems

#endif  // RCTEMS_COMMON_LOGGING_H_
