// Copyright 2022 The Oak Authors.

#ifndef OAK_LOGGING_LOGGING_H_
#define OAK_LOGGING_LOGGING_H_

#include <stdlib.h>
#include <functional>
#include "oak/common/stringpiece.h"
#include "oak/logging/log_level.h"
#include "oak/common/macros.h"

#define OAK_LOG(level, ...) do {                      \
  oak::logging_internal::LogImpl(                     \
      oak::LogLevel::OAK_LOG_LEVEL_ ## level,         \
      oak::logging_internal::Basename(                \
          __FILE__, sizeof(__FILE__) - 1),            \
      __LINE__, __VA_ARGS__);                         \
  if (oak::LogLevel::OAK_LOG_LEVEL_ ## level ==       \
      oak::LogLevel::OAK_LOG_LEVEL_FATAL)             \
    abort();                                          \
} while (0)

#define OAK_DEBUG(...)    OAK_LOG(DEBUG, __VA_ARGS__)
#define OAK_INFO(...)     OAK_LOG(INFO, __VA_ARGS__)
#define OAK_WARNING(...)  OAK_LOG(WARNING, __VA_ARGS__)
#define OAK_ERROR(...)    OAK_LOG(ERROR, __VA_ARGS__)
#define OAK_FATAL(...)    OAK_LOG(FATAL, __VA_ARGS__)

namespace oak {

void RegisterLogger(std::function<void(StringPiece)>&& logger);
void SetupLogLevel(LogLevel level);

namespace logging_internal {
// Strip directory which any leading directory compoents has removed.
constexpr const char* Basename(const char* fname, int off) {
  return off == 0 || fname[off - 1] == '/'
      ? fname + off : Basename(fname, off - 1);
}

void LogImpl(LogLevel level, const char* fname, int line, const char* fmt, ...)
    OAK_ATTR_PRINTF(4, 5);
}  // namespace logging_internal
}  // namespace oak

#endif  // OAK_LOGGING_LOGGING_H_
