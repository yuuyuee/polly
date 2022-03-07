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
#include "polly/stubs/type_traits.h"

namespace polly {
struct enable_default_constructors_tag {
  explicit constexpr enable_default_constructors_tag() = default;
};

template <bool Switch, typename Tag = void>
struct enable_default_constructor {
  constexpr enable_default_constructor() noexcept = default;
  constexpr enable_default_constructor(enable_default_constructor const&) noexcept = default;
  constexpr enable_default_constructor(enable_default_constructor&&) noexcept = default;
  enable_default_constructor& operator=(enable_default_constructor const&) noexcept = default;
  enable_default_constructor& operator=(enable_default_constructor&&) noexcept = default;

  constexpr explicit enable_default_constructor(enable_default_constructors_tag) {}
};

template <typename Tag>
struct enable_default_constructor<false, Tag> {
  constexpr enable_default_constructor() noexcept = delete;
  constexpr enable_default_constructor(enable_default_constructor const&) noexcept = default;
  constexpr enable_default_constructor(enable_default_constructor&&) noexcept = default;
  enable_default_constructor& operator=(enable_default_constructor const&) noexcept = default;
  enable_default_constructor& operator=(enable_default_constructor&&) noexcept = default;

  constexpr explicit enable_default_constructor(enable_default_constructors_tag) {}
};

template <bool Switch, typename Tag = void>
struct enable_destructor {};

template <typename Tag>
struct enable_destructor<false, Tag> {
  ~enable_destructor() noexcept = default;
};

template <
    bool Copy, bool CopyAssignment,
    bool Move, bool MoveAssignment,
    typename Tag = void>
struct enable_copy_move {};

template <typename Tag>
struct enable_copy_move<false, true, true, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<true, false, true, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<false, false, true, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<true, true, false, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<false, true, false, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<true, false, false, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<false, false, false, true, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = default;
};

template <typename Tag>
struct enable_copy_move<true, true, true, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<false, true, true, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<true, false, true, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<false, false, true, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = default;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<true, true, false, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<false, true, false, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = default;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<true, false, false, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = default;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <typename Tag>
struct enable_copy_move<false, false, false, false, Tag> {
  constexpr enable_copy_move() noexcept                         = default;
  constexpr enable_copy_move(const enable_copy_move&) noexcept  = delete;
  constexpr enable_copy_move(enable_copy_move&&) noexcept       = delete;
  enable_copy_move& operator=(const enable_copy_move&) noexcept = delete;
  enable_copy_move& operator=(enable_copy_move&&) noexcept      = delete;
};

template <
    bool Default, bool Destructor,
    bool Copy, bool CopyAssignment,
    bool Move, bool MoveAssignment,
    typename Tag = void>
struct enable_special_members
    : private enable_default_constructor<Default, Tag>,
      private enable_destructor<Destructor, Tag>,
      private enable_copy_move<Copy, CopyAssignment, Move, MoveAssignment, Tag> {};

} // namespace polly