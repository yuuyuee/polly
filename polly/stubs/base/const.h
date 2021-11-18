#pragma once

#include "stubs/base/config.h"

namespace polly {
namespace const_internal {
constexpr const char* Endspoint(const char* s) {
  return !!*s ? Endspoint(s + 1) : s;
}

constexpr const char* ReverseFindFirst(const char* b, const char* e, char c) {
  return b == e ? e : *b == c ? b : ReverseFindFirst(b - 1, e, c);
}
} // namespace const_internal

constexpr size_t ConstStrlen(const char* s) {
  return const_internal::Endspoint(s) - s;
}

constexpr const char* ConstBasename(const char* s) {
  return s - 1 == const_internal::ReverseFindFirst(s + ConstStrlen(s) - 1, s - 1, '/')
      ? s : const_internal::ReverseFindFirst(s + ConstStrlen(s) - 1, s - 1, '/');
}

} // namespace polly