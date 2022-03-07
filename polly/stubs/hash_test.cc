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

#include "polly/stubs/hash.h"

#include <string>
#include <functional>
#include "gtest/gtest.h"

TEST(HashTest, MurMurHash64) {
  std::string s1("string");
  std::string s2 = s1;
  std::string empty;
  EXPECT_EQ(polly::MurMurHash64(s1.c_str(), s1.size()), polly::MurMurHash64(s2.c_str(), s2.size()));
  EXPECT_NE(polly::MurMurHash64(s1.c_str(), s1.size()), polly::MurMurHash64(empty.c_str(), empty.size()));
  EXPECT_EQ(polly::MurMurHash64(s1.c_str(), s1.size()), std::hash<std::string>{}(s1));
  EXPECT_EQ(polly::MurMurHash64(empty.c_str(), empty.size()), 0);
}

TEST(HashTest, FNVHash64) {
  const char* p0 = "strings";
  const char* p1 = "strings";
  const char* p2 = "numbers";
  EXPECT_EQ(polly::FNVHash64(p0, strlen(p0)), polly::FNVHash64(p1, strlen(p1)));
  EXPECT_NE(polly::FNVHash64(p0, strlen(p0)), polly::FNVHash64(p2, strlen(p2)));
  EXPECT_EQ(polly::FNVHash64("", 0), 0);
}