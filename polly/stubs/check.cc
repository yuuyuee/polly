#include "stubs/check.h"

#include <unistd.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>

namespace polly {
namespace check_internal {
void SafeWriteToStderr(const char* file, int line, const char* fmt, ...) {
  constexpr const char* kTruncated = " ... (message truncated)\n";
  constexpr size_t kBufferSize = 1024;
  constexpr size_t kTruncatedOffset = kBufferSize - polly::ConstStrlen(kTruncated) - 1;
  char buffer[kBufferSize];

  int prefix_len = snprintf(buffer, kBufferSize, "[%s:%d] ", file, line);

  va_list ap;
  va_start(ap, fmt);
  int len = ::vsnprintf(&(buffer[0]) + prefix_len, kBufferSize - prefix_len, fmt, ap);
  va_end(ap);
  if (len <= 0)
    return;

  len += prefix_len;
  if (len >= static_cast<int>(kBufferSize)) {
    memcpy(&(buffer[0]) + kTruncatedOffset, kTruncated, polly::ConstStrlen(kTruncated));
    len = kBufferSize - 1;
  }
  ::write(STDERR_FILENO, buffer, len);
}
} // namespace check_internal
} // namespace polly