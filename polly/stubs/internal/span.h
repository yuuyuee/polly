
//
// Copyright 2019 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <algorithm>
#include <cstddef>
#include <string>

#include "polly/stubs/internal/throw_delegate.h"
#include "polly/stubs/type_traits.h"

namespace polly {
namespace span_internal {
// A constexpr min function
constexpr size_t Min(size_t a, size_t b) noexcept { return a < b ? a : b; }

// Wrappers for access to container data pointers.
template <typename C>
constexpr auto GetDataImpl(C& c, char) noexcept -> decltype(c.data()) {
  return c.data();
}

// Before C++17, std::string::data returns a const char* in all cases.
inline char* GetDataImpl(std::string& s, int) noexcept {
  return &s[0];
}

template <typename C>
constexpr auto GetData(C& c) noexcept -> decltype(GetDataImpl(c, 0)) {
  return GetDataImpl(c, 0);
}

// Detection idioms for size() and data().
template <typename C>
using HasSize =
    std::is_integral<polly::decay_t<decltype(std::declval<C&>().size())>>;

// We want to enable conversion from vector<T*> to Span<const T* const> but
// disable conversion from vector<Derived> to Span<Base>. Here we use
// the fact that U** is convertible to Q* const* if and only if Q is the same
// type or a more cv-qualified version of U.  We also decay the result type of
// data() to avoid problems with classes which have a member function data()
// which returns a reference.
template <typename Tp, typename C>
using HasData =
    std::is_convertible<polly::decay_t<decltype(GetData(std::declval<C&>()))>*,
                        Tp* const*>;

// Extracts value type from a Container
template <typename C>
struct ElementType {
  using type = typename std::remove_reference<C>::type::value_type;
};

template <typename Tp, size_t N>
struct ElementType<Tp (&)[N]> {
  using type = Tp;
};

template <typename C>
using ElementT = typename ElementType<C>::type;

template <typename T>
using EnableIfMutable =
    typename std::enable_if<!std::is_const<T>::value, int>::type;

template <template <typename> class SpanT, typename Tp>
bool EqualImpl(SpanT<Tp> a, SpanT<Tp> b) {
  static_assert(std::is_const<Tp>::value, "");
  return std::distance(a.begin(), a.end()) == std::distance(b.begin(), b.end())
      ? std::equal(a.begin(), a.end(), b.begin()) : false;
}

template <template <typename> class SpanT, typename Tp>
bool LessThanImpl(SpanT<Tp> a, SpanT<Tp> b) {
  static_assert(std::is_const<Tp>::value, "");
  return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end());
}

template <typename From, typename To>
using EnableIfConvertibleTo =
    typename std::enable_if<std::is_convertible<From, To>::value>::type;
}  // namespace span_internal
}  // namespace polly
