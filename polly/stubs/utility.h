#pragma once

#include <utility>

#include "stubs/internal/config.h"
#include "stubs/macros.h"

namespace polly {

#if defined(POLLY_HAVE_STD_OPTIONAL)
using std::in_place_t;
using std::in_place;
#else // POLLY_HAVE_STD_OPTIONAL
struct in_place_t {};
POLLY_INLINE_CONSTEXPR(in_place_t, in_place,);
#endif // POLLY_HAVE_STD_OPTIONAL

#if defined(POLLY_HAVE_STD_ANY) || defined(POLLY_HAVE_STD_VARIANT)
using std::in_place_type;
using std::in_place_type_t;
#else // POLLY_HAVE_STD_ANY || POLLY_HAVE_STD_VARIANT
template<typename T> struct in_place_type_t {
  explicit in_place_type_t() = default;
};

template<typename T>
void in_place_type(in_place_type_t<T>) {}
#endif // POLLY_HAVE_STD_ANY || POLLY_HAVE_STD_VARIANT

#ifdef POLLY_HAVE_STD_VARIANT
using std::in_place_index;
using std::in_place_index_t;
#else
template<size_t I> struct in_place_index_t {
  explicit in_place_index_t() = default;
};

template<size_t I>
void in_place_index(in_place_index_t<I>) {}
#endif  // ABSL_USES_STD_VARIANT

// Like as std::min but constant function.
template<typename Tp>
constexpr const Tp& min(const Tp& a, const Tp& b) {
  return b < a ? b : a;
}

// Like as std::max but constant function.
template<typename Tp>
constexpr const Tp& max(const Tp& a, const Tp& b) {
  return a < b ? b: a;
}


} // namespace polly