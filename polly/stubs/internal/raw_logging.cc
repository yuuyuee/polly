#include "stubs/internal/raw_logging.h"

#include <unistd.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace polly {
namespace raw_logging_internal {

void RawLog(const char* fmt, ...) {
  constexpr const char kTruncated[] = " ... (message truncated)\n";
  constexpr const size_t kBufferSize = 256;
  constexpr size_t kOffset = kBufferSize - sizeof(kTruncated);
  char buffer[kBufferSize];

  va_list ap;
  va_start(ap, fmt);
  int len = ::vsnprintf(buffer, kBufferSize, fmt, ap);
  va_end(ap);

  if (len < 0)
    len = 0;

  if (len >= static_cast<int>(kBufferSize)) {
    memcpy(buffer + kOffset, kTruncated, sizeof(kTruncated));
    len = kBufferSize - 1;
  }

  ::write(STDERR_FILENO, buffer, len);
}

} // namespace raw_logging_internal
} // namespace polly