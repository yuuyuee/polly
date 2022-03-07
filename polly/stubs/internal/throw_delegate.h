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
#include <string>

namespace polly {
namespace throw_delegate_internal {

// std::logic_error
[[noreturn]] void ThrowStdLogicError(const std::string& what);
[[noreturn]] void ThrowStdLogicError(const char* what);

// std::domain_error
[[noreturn]] void ThrowStdDomainError(const std::string& what);
[[noreturn]] void ThrowStdDomainError(const char* what);

// std::invalid_argument
[[noreturn]] void ThrowStdInvalidArgument(const std::string& what);
[[noreturn]] void ThrowStdInvalidArgument(const char* what);

// std::length_error
[[noreturn]] void ThrowStdLengthError(const std::string& what);
[[noreturn]] void ThrowStdLengthError(const char* what);

// std::out_of_range
[[noreturn]] void ThrowStdOutOfRange(const std::string& what);
[[noreturn]] void ThrowStdOutOfRange(const char* what);

// std::runtime_error
[[noreturn]] void ThrowStdRuntimeError(const std::string& what);
[[noreturn]] void ThrowStdRuntimeError(const char* what);

// std::range_error
[[noreturn]] void ThrowStdRangeError(const std::string& what);
[[noreturn]] void ThrowStdRangeError(const char* what);

// std::overflow_error
[[noreturn]] void ThrowStdOverflowError(const std::string& what);
[[noreturn]] void ThrowStdOverflowError(const char* what);

// std::underflow_error
[[noreturn]] void ThrowStdUnderflowError(const std::string& what);
[[noreturn]] void ThrowStdUnderflowError(const char* what);

} // namespace throw_delegate_internal
} // namespace polly