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

#include "polly/stubs/internal/raw_logging.h"

#include <unistd.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace polly {
namespace raw_logging_internal {

void RawLog(const char* fname, int line, const char* severity, const char* fmt, ...) {
  static const char kTruncated[] = " ... (message truncated)\n";
  static const size_t kBufferSize = 512;
  static const size_t kOffset = kBufferSize - sizeof(kTruncated);
  char buffer[kBufferSize];

  int len = ::snprintf(buffer, kBufferSize, "[%s:%d @ %-5s] ",
      fname, line, severity);
  va_list ap;
  va_start(ap, fmt);
  len = ::vsnprintf(buffer + len, kBufferSize - len, fmt, ap);
  va_end(ap);

  if (len < 0)
    return;

  if (len >= static_cast<int>(kBufferSize)) {
    memcpy(buffer + kOffset, kTruncated, sizeof(kTruncated));
    len = kBufferSize - 1;
  }

  ::write(STDERR_FILENO, buffer, len);
}

} // namespace raw_logging_internal
} // namespace polly