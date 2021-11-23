#include "stubs/hash.h"

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