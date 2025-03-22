// Copyright 2022 The Oak Authors.

#include "oak/logging/logging.h"

#include <unistd.h>
#include <stdarg.h>
#include <utility>

namespace oak {
namespace logging_internal {
namespace {

constexpr const char kTailMsg[] = "... (message truncated)\n";
constexpr const size_t kTailMsgSize = sizeof(kTailMsg);

void DefaultLogger(StringPiece msg) {
  IGNORE_UNUESD(write(STDERR_FILENO, msg.data(), msg.size()));
}

std::function<void(StringPiece)> kLogger(DefaultLogger);
LogLevel kLogLevel = LogLevel::OAK_LOG_LEVEL_ERROR;
}  // anonymous namespace

void LogImpl(LogLevel level, const char* fname, int line, const char* fmt, ...) {
  constexpr const int kBufferSize = 2048;
  char buffer[kBufferSize];

  if (level < kLogLevel)
    return;

  int plen = snprintf(buffer, kBufferSize, "%s [%s:%d] ",
      LogLevelShortName(level), fname, line);

  va_list ap;
  va_start(ap, fmt);
  int len = vsnprintf(buffer + plen, kBufferSize - plen, fmt, ap);
  if (len < 0) len = 0;
  if (len >= kBufferSize - plen) {
    memcpy(buffer + kBufferSize - kTailMsgSize, kTailMsg, kTailMsgSize);
    // snprintf and vsnprintf write at most size bytes inlcude the
    // terminating null byte '\0'.
    len = kBufferSize - plen - 1;
  }
  va_end(ap);

  if (kLogger) {
    kLogger({buffer, static_cast<size_t>(plen + len)});
  } else {
    DefaultLogger({buffer, static_cast<size_t>(plen + len)});
  }
}
}  // namespace logging_internal

void RegisterLogger(std::function<void(StringPiece)>&& logger) {
  logging_internal::kLogger = std::move(logger);
}

void SetupLogLevel(LogLevel level) {
  logging_internal::kLogLevel = level;
}

}  // namespace oak
