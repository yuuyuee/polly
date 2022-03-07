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

#include <cstddef>

namespace polly {
// Implementation of Murmur hash for 64-bit size_t.
size_t MurMurHash64(const void* ptr, size_t len, size_t seed = 0xC70F6907UL);

// Implementation of FNV-1a hash for 64-bit size_t.
size_t FNVHash64(const void* ptr, size_t len, size_t seed = 0x811C9DC5UL);

}  // namespace std