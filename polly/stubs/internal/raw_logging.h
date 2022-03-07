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

#include "polly/stubs/internal/config.h"

#include <cassert>
#include <cstdlib>

#define POLLY_RAW_LOGGING_INFO ::polly::raw_logging_internal::LogSeverity::kInfo
#define POLLY_RAW_LOGGING_WARN ::polly::raw_logging_internal::LogSeverity::kWarning
#define POLLY_RAW_LOGGING_ERROR ::polly::raw_logging_internal::LogSeverity::kError
#define POLLY_RAW_LOGGING_FATAL ::polly::raw_logging_internal::LogSeverity::kFatal

#define POLLY_RAW_LOG(severity, fmt, ...) do {                      \
  ::polly::raw_logging_internal::RawLog(                            \
      ::polly::raw_logging_internal::Basename(                      \
          __FILE__, sizeof(__FILE__) - 1),                          \
      __LINE__,                                                     \
      ::polly::raw_logging_internal::StrLogSeverity(                \
          POLLY_RAW_LOGGING_ ## severity),                          \
      fmt,                                                          \
      ## __VA_ARGS__);                                              \
                                                                    \
  if (POLLY_RAW_LOGGING_ ## severity == POLLY_RAW_LOGGING_FATAL) {  \
    ::abort();                                                      \
  }                                                                 \
} while (0)

# define POLLY_RAW_CHECK(exp, msg) do {                             \
  if (POLLY_EXPECT_FALSE(!(exp))) {                                 \
    POLLY_RAW_LOG(FATAL, "Check "  #exp " failed: %s", msg);        \
  }                                                                 \
} while (0)

namespace polly {
namespace raw_logging_internal {

enum class LogSeverity {
  kInfo = 0,
  kWarning,
  kError,
  kFatal
};

constexpr const char* StrLogSeverity(LogSeverity severity) {
  return severity == LogSeverity::kInfo ? "INFO"
      : severity == LogSeverity::kWarning ? "WARN"
          : severity == LogSeverity::kError ? "ERROR"
              : severity == LogSeverity::kFatal ? "FATAL"
                  : "UNKNOWN";
}

constexpr const char* Basename(const char* fname, int offset) {
  return offset == 0 || fname[offset - 1] == '/'
      ? fname + offset : Basename(fname,  offset - 1);
}

void RawLog(const char* fname, int line, const char* severity, const char* fmt, ...);

} // namespace raw_logging_internal
} // namespace polly