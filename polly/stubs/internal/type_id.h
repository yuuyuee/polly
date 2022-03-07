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

namespace polly {
namespace type_id_internal {
template <typename Tp>
struct TypeTag {
  static constexpr const char kId = 0;
};

template <typename Tp>
constexpr const char TypeTag<Tp>::kId;

using TypeIdType = const void*;

template <typename Tp>
constexpr inline TypeIdType TypeId() {
  return &TypeTag<Tp>::kId;
}

} // namespace type_id_internal
} // namespace polly
