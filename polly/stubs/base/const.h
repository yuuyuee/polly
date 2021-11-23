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

template<typename T>
constexpr const T& ConstMin(const T& a, const T& b) {
  return b < a ? b : a;
}

template<typename T, typename Compare>
constexpr const T& ConstMin(const T& a, const T& b, Compare compare) {
  return compare(b, a) ? b : a;
}

template<typename T>
constexpr const T& ConstMax(const T& a, const T& b) {
  return a < b ? b: a;
}

template<typename T, typename Compare>
constexpr const T& ConstMax(const T& a, const T& b, Compare compare) {
  return  compare(a, b) ? b: a;
}

} // namespace polly