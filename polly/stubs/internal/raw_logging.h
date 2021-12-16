#pragma once

#include <cassert>
#include <cstdlib>
#include "stubs/internal/config.h"

#define POLLY_RAW_LOGGING_INFO ::polly::raw_logging_internal::LogSeverity::kInfo
#define POLLY_RAW_LOGGING_WARN ::polly::raw_logging_internal::LogSeverity::kWarning
#define POLLY_RAW_LOGGING_ERROR ::polly::raw_logging_internal::LogSeverity::kError
#define POLLY_RAW_LOGGING_FATAL ::polly::raw_logging_internal::LogSeverity::kFatal

#define POLLY_INTERNAL_LOG(severity, message) do {                  \
  ::polly::raw_logging_internal::RawLog(                            \
      "[%s@%d %-4s] %s\n",                                          \
      ::polly::raw_logging_internal::Basename(__FILE__),            \
      __LINE__,                                                     \
      POLLY_RAW_LOGGING_ ## severity,                               \
      message                                                       \
  );                                                                \
                                                                    \
  if (POLLY_RAW_LOGGING_ ## severity == POLLY_RAW_LOGGING_FATAL) {  \
    ::abort();                                                      \
  }                                                                 \
} while (0)

#define POLLY_INTERNAL_LOG_INFO(message) POLLY_INTERNAL_LOG(INFO, message)
#define POLLY_INTERNAL_LOG_WARN(message) POLLY_INTERNAL_LOG(WARN, message)
#define POLLY_INTERNAL_LOG_ERROR(message) POLLY_INTERNAL_LOG(ERROR, message)
#define POLLY_INTERNAL_LOG_FATAL(message) POLLY_INTERNAL_LOG(FATAL, message)

#if defined(NDEBUG)

# define POLLY_CONST_ASSERT(exp) (false ? static_cast<void>(exp) : static_cast<void>(0))
# define POLLY_ASSERT(exp) (false ? static_cast<void>(exp) : static_cast<void>(0))

#else

# define POLLY_CONST_ASSERT(exp)  \
    (POLLY_EXPECT_TRUE(exp) ? static_cast<void>(0) : [] { assert(false && #exp); }())

# define POLLY_ASSERT(exp) do {                 \
  if (POLLY_EXPECT_FALSE(!(exp))) {             \
    POLLY_LOG_FATAL("Check "  #exp " failed");  \
  }                                             \
} while (0)

#endif // NDEBUG

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

void RawLog(const char* fmt, ...);

} // namespace raw_logging_internal
} // namespace polly