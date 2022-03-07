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

#include <utility>
#include "polly/stubs/internal/inline_variable.h"

namespace polly {

#if defined(POLLY_HAVE_STD_OPTIONAL)
using std::in_place_t;
using std::in_place;
#else // POLLY_HAVE_STD_OPTIONAL
struct in_place_t {
  explicit in_place_t() = default;
};
POLLY_INLINE_CONSTEXPR(polly::in_place_t, in_place, {});
#endif // POLLY_HAVE_STD_OPTIONAL

#if defined(POLLY_HAVE_STD_ANY) || defined(POLLY_HAVE_STD_VARIANT)
using std::in_place_type;
using std::in_place_type_t;
#else // POLLY_HAVE_STD_ANY || POLLY_HAVE_STD_VARIANT
namespace utility_internal {
template <typename Tp>
struct in_place_type_tag {
  explicit in_place_type_tag() = delete;
  in_place_type_tag(const in_place_type_tag&) = delete;
  in_place_type_tag& operator=(const in_place_type_tag&) = delete;
};
} // namespace utility_internal

template<typename Tp>
using in_place_type_t = void(*)(utility_internal::in_place_type_tag<Tp>);

template<typename Tp>
void in_place_type(utility_internal::in_place_type_tag<Tp>) {}
#endif // POLLY_HAVE_STD_ANY || POLLY_HAVE_STD_VARIANT

#ifdef POLLY_HAVE_STD_VARIANT
using std::in_place_index;
using std::in_place_index_t;
#else // POLLY_HAVE_STD_VARIANT
namespace utility_internal {
template <size_t N>
struct in_place_index_tag {
  explicit in_place_index_tag() = delete;
  in_place_index_tag(const in_place_index_tag&) = delete;
  in_place_index_tag& operator=(const in_place_index_tag&) = delete;
};
} // namespace utility_internal

template<size_t N>
using in_place_index_t = void (*)(utility_internal::in_place_index_tag<N>);

template<size_t N>
void in_place_index(utility_internal::in_place_index_tag<N>) {}
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

// Forms lvalue reference to const type of Tp.
template <typename Tp>
constexpr typename std::add_const<Tp>::type& as_const(Tp& v) noexcept {
  return v;
}

// const rvalue reference overload is deleted to disallow rvalue arguments.
template <typename Tp>
void as_const(Tp&& v) = delete;


} // namespace polly
