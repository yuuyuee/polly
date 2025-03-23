// Copyright 2022 The Oak Authors.

#include "oak/common/stringpiece.h"

#include <map>
#include "gtest/gtest.h"

using oak::StringPiece;

namespace {

TEST(StringViewTest, Ctor) {
  {
    // Null.
    StringPiece s10;
    EXPECT_TRUE(s10.data() == nullptr);
    EXPECT_EQ(0U, s10.length());
  }

  {
    // const char* without length.
    const char* hello = "hello";
    StringPiece v1(hello);
    EXPECT_TRUE(v1.data() == hello);
    EXPECT_EQ(5U, v1.length());

    // const char* with length.
    StringPiece v2(hello, 4);
    EXPECT_TRUE(v2.data() == hello);
    EXPECT_EQ(4U, v2.length());

    // Not recommended, but valid C++
    StringPiece v3(hello, 6);
    EXPECT_TRUE(v3.data() == hello);
    EXPECT_EQ(6U, v3.length());
  }

  {
    // std::string.
    std::string hola = "hola";
    StringPiece v1(hola);
    EXPECT_TRUE(v1.data() == hola.data());
    EXPECT_EQ(4U, v1.length());

    // std::string with embedded '\0'.
    hola.push_back('\0');
    hola.append("h2");
    hola.push_back('\0');
    StringPiece v2(hola);
    EXPECT_TRUE(v2.data() == hola.data());
    EXPECT_EQ(8U, v2.length());
  }
}

TEST(StringViewTest, Swap) {
  StringPiece a("a");
  StringPiece b("bbb");
  EXPECT_TRUE(noexcept(a.swap(b)));
  a.swap(b);
  EXPECT_EQ(a, "bbb");
  EXPECT_EQ(b, "a");
  a.swap(b);
  EXPECT_EQ(a, "a");
  EXPECT_EQ(b, "bbb");
}

TEST(StringViewTest, STLComparator) {
  std::string s1("foo");
  std::string s2("bar");
  std::string s3("baz");

  StringPiece p1(s1);
  StringPiece p2(s2);
  StringPiece p3(s3);

  typedef std::map<StringPiece, int> TestMap;
  TestMap map;

  map.insert(std::make_pair(p1, 0));
  map.insert(std::make_pair(p2, 1));
  map.insert(std::make_pair(p3, 2));
  EXPECT_EQ(map.size(), 3U);

  TestMap::const_iterator iter = map.begin();
  EXPECT_EQ(iter->second, 1);
  ++iter;
  EXPECT_EQ(iter->second, 2);
  ++iter;
  EXPECT_EQ(iter->second, 0);
  ++iter;
  EXPECT_TRUE(iter == map.end());

  TestMap::iterator new_iter = map.find("zot");
  EXPECT_TRUE(new_iter == map.end());

  new_iter = map.find("bar");
  EXPECT_TRUE(new_iter != map.end());

  map.erase(new_iter);
  EXPECT_EQ(map.size(), 2U);

  iter = map.begin();
  EXPECT_EQ(iter->second, 2);
  ++iter;
  EXPECT_EQ(iter->second, 0);
  ++iter;
  EXPECT_TRUE(iter == map.end());
}

#define COMPARE(result, op, x, y)                                     \
  EXPECT_EQ(result, StringPiece((x)) op StringPiece((y)));  \
  EXPECT_EQ(result, StringPiece((x)).compare(StringPiece((y))) op 0)

TEST(StringViewTest, ComparisonOperators) {
  COMPARE(true, ==, "",   "");
  COMPARE(true, ==, "", StringPiece());
  COMPARE(true, ==, StringPiece(), "");
  COMPARE(true, ==, "a",  "a");
  COMPARE(true, ==, "aa", "aa");
  COMPARE(false, ==, "a",  "");
  COMPARE(false, ==, "",   "a");
  COMPARE(false, ==, "a",  "b");
  COMPARE(false, ==, "a",  "aa");
  COMPARE(false, ==, "aa", "a");

  COMPARE(false, !=, "",   "");
  COMPARE(false, !=, "a",  "a");
  COMPARE(false, !=, "aa", "aa");
  COMPARE(true, !=, "a",  "");
  COMPARE(true, !=, "",   "a");
  COMPARE(true, !=, "a",  "b");
  COMPARE(true, !=, "a",  "aa");
  COMPARE(true, !=, "aa", "a");

  COMPARE(true, <, "a",  "b");
  COMPARE(true, <, "a",  "aa");
  COMPARE(true, <, "aa", "b");
  COMPARE(true, <, "aa", "bb");
  COMPARE(false, <, "a",  "a");
  COMPARE(false, <, "b",  "a");
  COMPARE(false, <, "aa", "a");
  COMPARE(false, <, "b",  "aa");
  COMPARE(false, <, "bb", "aa");

  COMPARE(true, <=, "a",  "a");
  COMPARE(true, <=, "a",  "b");
  COMPARE(true, <=, "a",  "aa");
  COMPARE(true, <=, "aa", "b");
  COMPARE(true, <=, "aa", "bb");
  COMPARE(false, <=, "b",  "a");
  COMPARE(false, <=, "aa", "a");
  COMPARE(false, <=, "b",  "aa");
  COMPARE(false, <=, "bb", "aa");

  COMPARE(false, >=, "a",  "b");
  COMPARE(false, >=, "a",  "aa");
  COMPARE(false, >=, "aa", "b");
  COMPARE(false, >=, "aa", "bb");
  COMPARE(true, >=, "a",  "a");
  COMPARE(true, >=, "b",  "a");
  COMPARE(true, >=, "aa", "a");
  COMPARE(true, >=, "b",  "aa");
  COMPARE(true, >=, "bb", "aa");

  COMPARE(false, >, "a",  "a");
  COMPARE(false, >, "a",  "b");
  COMPARE(false, >, "a",  "aa");
  COMPARE(false, >, "aa", "b");
  COMPARE(false, >, "aa", "bb");
  COMPARE(true, >, "b",  "a");
  COMPARE(true, >, "aa", "a");
  COMPARE(true, >, "b",  "aa");
  COMPARE(true, >, "bb", "aa");
}

TEST(StringViewTest, ComparisonOperatorsByCharacterPosition) {
  std::string x;
  for (int i = 0; i < 256; i++) {
    x += 'a';
    std::string y = x;
    COMPARE(true, ==, x, y);
    for (int j = 0; j < i; j++) {
      std::string z = x;
      z[j] = 'b';       // Differs in position 'j'
      COMPARE(false, ==, x, z);
      COMPARE(true, <, x, z);
      COMPARE(true, >, z, x);
      if (j + 1 < i) {
        z[j + 1] = 'A';  // Differs in position 'j+1' as well
        COMPARE(false, ==, x, z);
        COMPARE(true, <, x, z);
        COMPARE(true, >, z, x);
        z[j + 1] = 'z';  // Differs in position 'j+1' as well
        COMPARE(false, ==, x, z);
        COMPARE(true, <, x, z);
        COMPARE(true, >, z, x);
      }
    }
  }
}
#undef COMPARE

template <typename T>
struct is_type {
  template <typename U>
  static bool same(U) {
    return false;
  }
  static bool same(T) { return true; }
};

TEST(StringViewTest, NposMatchesStdStringView) {
  EXPECT_EQ(StringPiece::npos, std::string::npos);

  EXPECT_TRUE(is_type<size_t>::same(StringPiece::npos));
  EXPECT_FALSE(is_type<size_t>::same(""));

  // Make sure StringPiece::npos continues to be a header constant.
  char test[StringPiece::npos & 1] = {0};
  EXPECT_EQ(0, test[0]);
}

TEST(StringViewTest, STL1) {
  const StringPiece a("abcdefghijklmnopqrstuvwxyz");
  const StringPiece b("abc");
  const StringPiece c("xyz");
  const StringPiece d("foobar");
  const StringPiece e;
  std::string temp("123");
  temp += '\0';
  temp += "456";
  const StringPiece f(temp);

  EXPECT_EQ(a[6], 'g');
  EXPECT_EQ(b[0], 'a');
  EXPECT_EQ(c[2], 'z');
  EXPECT_EQ(f[3], '\0');
  EXPECT_EQ(f[5], '5');

  EXPECT_EQ(*d.data(), 'f');
  EXPECT_EQ(d.data()[5], 'r');
  EXPECT_TRUE(e.data() == nullptr);

  EXPECT_EQ(*a.begin(), 'a');
  EXPECT_EQ(*(b.begin() + 2), 'c');
  EXPECT_EQ(*(c.end() - 1), 'z');

  EXPECT_EQ(*a.rbegin(), 'z');
  EXPECT_EQ(*(b.rbegin() + 2), 'a');
  EXPECT_EQ(*(c.rend() - 1), 'x');
  EXPECT_TRUE(a.rbegin() + 26 == a.rend());

  EXPECT_EQ(a.size(), 26U);
  EXPECT_EQ(b.size(), 3U);
  EXPECT_EQ(c.size(), 3U);
  EXPECT_EQ(d.size(), 6U);
  EXPECT_EQ(e.size(), 0U);
  EXPECT_EQ(f.size(), 7U);

  EXPECT_TRUE(!d.empty());
  EXPECT_TRUE(d.begin() != d.end());
  EXPECT_TRUE(d.begin() + 6 == d.end());

  EXPECT_TRUE(e.empty());
  EXPECT_TRUE(e.begin() == e.end());

  char buf[4] = { '%', '%', '%', '%' };
  EXPECT_EQ(a.copy(buf, 4), 4U);
  EXPECT_EQ(buf[0], a[0]);
  EXPECT_EQ(buf[1], a[1]);
  EXPECT_EQ(buf[2], a[2]);
  EXPECT_EQ(buf[3], a[3]);
  EXPECT_EQ(a.copy(buf, 3, 7), 3U);
  EXPECT_EQ(buf[0], a[7]);
  EXPECT_EQ(buf[1], a[8]);
  EXPECT_EQ(buf[2], a[9]);
  EXPECT_EQ(buf[3], a[3]);
  EXPECT_EQ(c.copy(buf, 99), 3U);
  EXPECT_EQ(buf[0], c[0]);
  EXPECT_EQ(buf[1], c[1]);
  EXPECT_EQ(buf[2], c[2]);
  EXPECT_EQ(buf[3], a[3]);
  EXPECT_THROW(a.copy(buf, 1, 27), std::out_of_range);
}

// Separated from STL1() because some compilers produce an overly
// large stack frame for the combined function.
TEST(StringViewTest, STL2) {
  const StringPiece a("abcdefghijklmnopqrstuvwxyz");
  const StringPiece b("abc");
  const StringPiece c("xyz");
  StringPiece d("foobar");
  const StringPiece e;
  const StringPiece f(
      "123"
      "\0"
      "456",
      7);
  StringPiece g("xx not found bb");

  d = StringPiece();
  EXPECT_EQ(d.size(), 0U);
  EXPECT_TRUE(d.empty());
  EXPECT_TRUE(d.data() == nullptr);
  EXPECT_TRUE(d.begin() == d.end());

  EXPECT_EQ(a.find(b), 0U);
  EXPECT_EQ(a.find(b, 1), StringPiece::npos);
  EXPECT_EQ(a.find(c), 23U);
  EXPECT_EQ(a.find(c, 9), 23U);
  EXPECT_EQ(a.find(c, StringPiece::npos), StringPiece::npos);
  EXPECT_EQ(b.find(c), StringPiece::npos);
  EXPECT_EQ(b.find(c, StringPiece::npos), StringPiece::npos);
  EXPECT_EQ(a.find(d), 0U);
  EXPECT_EQ(a.find(e), 0U);
  EXPECT_EQ(a.find(d, 12), 12U);
  EXPECT_EQ(a.find(e, 17), 17U);
  EXPECT_EQ(a.find(g), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(d.find(b), StringPiece::npos);
  EXPECT_EQ(e.find(b), StringPiece::npos);
  EXPECT_EQ(d.find(b, 4), StringPiece::npos);
  EXPECT_EQ(e.find(b, 7), StringPiece::npos);

  size_t empty_search_pos = std::string().find(std::string());
  EXPECT_EQ(d.find(d), 0U);
  EXPECT_EQ(d.find(e), empty_search_pos);
  EXPECT_EQ(e.find(d), 0U);
  EXPECT_EQ(e.find(e), 0U);
  EXPECT_EQ(d.find(d, 4), std::string().find(std::string(), 4));
  EXPECT_EQ(d.find(e, 4), std::string().find(std::string(), 4));
  EXPECT_EQ(e.find(d, 4), std::string().find(std::string(), 4));
  EXPECT_EQ(e.find(e, 4), std::string().find(std::string(), 4));

  EXPECT_EQ(a.find('a'), 0U);
  EXPECT_EQ(a.find('c'), 2U);
  EXPECT_EQ(a.find('z'), 25U);
  EXPECT_EQ(a.find('$'), StringPiece::npos);
  EXPECT_EQ(a.find('\0'), StringPiece::npos);
  EXPECT_EQ(f.find('\0'), 3U);
  EXPECT_EQ(f.find('3'), 2U);
  EXPECT_EQ(f.find('5'), 5U);
  EXPECT_EQ(g.find('o'), 4U);
  EXPECT_EQ(g.find('o', 4), 4U);
  EXPECT_EQ(g.find('o', 5), 8U);
  EXPECT_EQ(a.find('b', 5), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(d.find('\0'), StringPiece::npos);
  EXPECT_EQ(e.find('\0'), StringPiece::npos);
  EXPECT_EQ(d.find('\0', 4), StringPiece::npos);
  EXPECT_EQ(e.find('\0', 7), StringPiece::npos);
  EXPECT_EQ(d.find('x'), StringPiece::npos);
  EXPECT_EQ(e.find('x'), StringPiece::npos);
  EXPECT_EQ(d.find('x', 4), StringPiece::npos);
  EXPECT_EQ(e.find('x', 7), StringPiece::npos);

  EXPECT_EQ(a.find(b.data(), 1, 0), 1U);
  EXPECT_EQ(a.find(c.data(), 9, 0), 9U);
  EXPECT_EQ(a.find(c.data(), StringPiece::npos, 0),
            StringPiece::npos);
  EXPECT_EQ(b.find(c.data(), StringPiece::npos, 0),
            StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(d.find(b.data(), 4, 0), StringPiece::npos);
  EXPECT_EQ(e.find(b.data(), 7, 0), StringPiece::npos);

  EXPECT_EQ(a.find(b.data(), 1), StringPiece::npos);
  EXPECT_EQ(a.find(c.data(), 9), 23U);
  EXPECT_EQ(a.find(c.data(), StringPiece::npos), StringPiece::npos);
  EXPECT_EQ(b.find(c.data(), StringPiece::npos), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(d.find(b.data(), 4), StringPiece::npos);
  EXPECT_EQ(e.find(b.data(), 7), StringPiece::npos);

  EXPECT_EQ(a.rfind(b), 0U);
  EXPECT_EQ(a.rfind(b, 1), 0U);
  EXPECT_EQ(a.rfind(c), 23U);
  EXPECT_EQ(a.rfind(c, 22), StringPiece::npos);
  EXPECT_EQ(a.rfind(c, 1), StringPiece::npos);
  EXPECT_EQ(a.rfind(c, 0), StringPiece::npos);
  EXPECT_EQ(b.rfind(c), StringPiece::npos);
  EXPECT_EQ(b.rfind(c, 0), StringPiece::npos);
  EXPECT_EQ(a.rfind(d), std::string(a).rfind(std::string()));
  EXPECT_EQ(a.rfind(e), std::string(a).rfind(std::string()));
  EXPECT_EQ(a.rfind(d, 12), 12U);
  EXPECT_EQ(a.rfind(e, 17), 17U);
  EXPECT_EQ(a.rfind(g), StringPiece::npos);
  EXPECT_EQ(d.rfind(b), StringPiece::npos);
  EXPECT_EQ(e.rfind(b), StringPiece::npos);
  EXPECT_EQ(d.rfind(b, 4), StringPiece::npos);
  EXPECT_EQ(e.rfind(b, 7), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(d.rfind(d, 4), std::string().rfind(std::string()));
  EXPECT_EQ(e.rfind(d, 7), std::string().rfind(std::string()));
  EXPECT_EQ(d.rfind(e, 4), std::string().rfind(std::string()));
  EXPECT_EQ(e.rfind(e, 7), std::string().rfind(std::string()));
  EXPECT_EQ(d.rfind(d), std::string().rfind(std::string()));
  EXPECT_EQ(e.rfind(d), std::string().rfind(std::string()));
  EXPECT_EQ(d.rfind(e), std::string().rfind(std::string()));
  EXPECT_EQ(e.rfind(e), std::string().rfind(std::string()));

  // g = "xx not found bb"
  EXPECT_EQ(g.rfind('o'), 8U);
  EXPECT_EQ(g.rfind('q'), StringPiece::npos);
  EXPECT_EQ(g.rfind('o', 8), 8U);
  EXPECT_EQ(g.rfind('o', 7), 4U);
  EXPECT_EQ(g.rfind('o', 3), StringPiece::npos);
  EXPECT_EQ(f.rfind('\0'), 3U);
  EXPECT_EQ(f.rfind('\0', 12), 3U);
  EXPECT_EQ(f.rfind('3'), 2U);
  EXPECT_EQ(f.rfind('5'), 5U);
  // empty string nonsense
  EXPECT_EQ(d.rfind('o'), StringPiece::npos);
  EXPECT_EQ(e.rfind('o'), StringPiece::npos);
  EXPECT_EQ(d.rfind('o', 4), StringPiece::npos);
  EXPECT_EQ(e.rfind('o', 7), StringPiece::npos);

  EXPECT_EQ(a.rfind(b.data(), 1, 0), 1U);
  EXPECT_EQ(a.rfind(c.data(), 22, 0), 22U);
  EXPECT_EQ(a.rfind(c.data(), 1, 0), 1U);
  EXPECT_EQ(a.rfind(c.data(), 0, 0), 0U);
  EXPECT_EQ(b.rfind(c.data(), 0, 0), 0U);
  EXPECT_EQ(d.rfind(b.data(), 4, 0), 0U);
  EXPECT_EQ(e.rfind(b.data(), 7, 0), 0U);
}

// Continued from STL2
TEST(StringViewTest, STL2FindFirst) {
  const StringPiece a("abcdefghijklmnopqrstuvwxyz");
  const StringPiece b("abc");
  const StringPiece c("xyz");
  StringPiece d("foobar");
  const StringPiece e;
  const StringPiece f(
      "123"
      "\0"
      "456",
      7);
  StringPiece g("xx not found bb");

  d = StringPiece();
  EXPECT_EQ(a.find_first_of(b), 0U);
  EXPECT_EQ(a.find_first_of(b, 0), 0U);
  EXPECT_EQ(a.find_first_of(b, 1), 1U);
  EXPECT_EQ(a.find_first_of(b, 2), 2U);
  EXPECT_EQ(a.find_first_of(b, 3), StringPiece::npos);
  EXPECT_EQ(a.find_first_of(c), 23U);
  EXPECT_EQ(a.find_first_of(c, 23), 23U);
  EXPECT_EQ(a.find_first_of(c, 24), 24U);
  EXPECT_EQ(a.find_first_of(c, 25), 25U);
  EXPECT_EQ(a.find_first_of(c, 26), StringPiece::npos);
  EXPECT_EQ(g.find_first_of(b), 13U);
  EXPECT_EQ(g.find_first_of(c), 0U);
  EXPECT_EQ(a.find_first_of(f), StringPiece::npos);
  EXPECT_EQ(f.find_first_of(a), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(a.find_first_of(d), StringPiece::npos);
  EXPECT_EQ(a.find_first_of(e), StringPiece::npos);
  EXPECT_EQ(d.find_first_of(b), StringPiece::npos);
  EXPECT_EQ(e.find_first_of(b), StringPiece::npos);
  EXPECT_EQ(d.find_first_of(d), StringPiece::npos);
  EXPECT_EQ(e.find_first_of(d), StringPiece::npos);
  EXPECT_EQ(d.find_first_of(e), StringPiece::npos);
  EXPECT_EQ(e.find_first_of(e), StringPiece::npos);

  EXPECT_EQ(a.find_first_not_of(b), 3U);
  EXPECT_EQ(a.find_first_not_of(c), 0U);
  EXPECT_EQ(b.find_first_not_of(a), StringPiece::npos);
  EXPECT_EQ(c.find_first_not_of(a), StringPiece::npos);
  EXPECT_EQ(f.find_first_not_of(a), 0U);
  EXPECT_EQ(a.find_first_not_of(f), 0U);
  EXPECT_EQ(a.find_first_not_of(d), 0U);
  EXPECT_EQ(a.find_first_not_of(e), 0U);
  // empty string nonsense
  EXPECT_EQ(a.find_first_not_of(d), 0U);
  EXPECT_EQ(a.find_first_not_of(e), 0U);
  EXPECT_EQ(a.find_first_not_of(d, 1), 1U);
  EXPECT_EQ(a.find_first_not_of(e, 1), 1U);
  EXPECT_EQ(a.find_first_not_of(d, a.size() - 1), a.size() - 1);
  EXPECT_EQ(a.find_first_not_of(e, a.size() - 1), a.size() - 1);
  EXPECT_EQ(a.find_first_not_of(d, a.size()), StringPiece::npos);
  EXPECT_EQ(a.find_first_not_of(e, a.size()), StringPiece::npos);
  EXPECT_EQ(a.find_first_not_of(d, StringPiece::npos),
            StringPiece::npos);
  EXPECT_EQ(a.find_first_not_of(e, StringPiece::npos),
            StringPiece::npos);
  EXPECT_EQ(d.find_first_not_of(a), StringPiece::npos);
  EXPECT_EQ(e.find_first_not_of(a), StringPiece::npos);
  EXPECT_EQ(d.find_first_not_of(d), StringPiece::npos);
  EXPECT_EQ(e.find_first_not_of(d), StringPiece::npos);
  EXPECT_EQ(d.find_first_not_of(e), StringPiece::npos);
  EXPECT_EQ(e.find_first_not_of(e), StringPiece::npos);

  StringPiece h("====");
  EXPECT_EQ(h.find_first_not_of('='), StringPiece::npos);
  EXPECT_EQ(h.find_first_not_of('=', 3), StringPiece::npos);
  EXPECT_EQ(h.find_first_not_of('\0'), 0U);
  EXPECT_EQ(g.find_first_not_of('x'), 2U);
  EXPECT_EQ(f.find_first_not_of('\0'), 0U);
  EXPECT_EQ(f.find_first_not_of('\0', 3), 4U);
  EXPECT_EQ(f.find_first_not_of('\0', 2), 2U);
  // empty string nonsense
  EXPECT_EQ(d.find_first_not_of('x'), StringPiece::npos);
  EXPECT_EQ(e.find_first_not_of('x'), StringPiece::npos);
  EXPECT_EQ(d.find_first_not_of('\0'), StringPiece::npos);
  EXPECT_EQ(e.find_first_not_of('\0'), StringPiece::npos);
}

// Continued from STL2
TEST(StringViewTest, STL2FindLast) {
  const StringPiece a("abcdefghijklmnopqrstuvwxyz");
  const StringPiece b("abc");
  const StringPiece c("xyz");
  StringPiece d("foobar");
  const StringPiece e;
  const StringPiece f(
      "123"
      "\0"
      "456",
      7);
  StringPiece g("xx not found bb");
  StringPiece h("====");
  StringPiece i("56");

  d = StringPiece();
  EXPECT_EQ(h.find_last_of(a), StringPiece::npos);
  EXPECT_EQ(g.find_last_of(a), g.size()-1);
  EXPECT_EQ(a.find_last_of(b), 2U);
  EXPECT_EQ(a.find_last_of(c), a.size()-1);
  EXPECT_EQ(f.find_last_of(i), 6U);
  EXPECT_EQ(a.find_last_of('a'), 0U);
  EXPECT_EQ(a.find_last_of('b'), 1U);
  EXPECT_EQ(a.find_last_of('z'), 25U);
  EXPECT_EQ(a.find_last_of('a', 5), 0U);
  EXPECT_EQ(a.find_last_of('b', 5), 1U);
  EXPECT_EQ(a.find_last_of('b', 0), StringPiece::npos);
  EXPECT_EQ(a.find_last_of('z', 25), 25U);
  EXPECT_EQ(a.find_last_of('z', 24), StringPiece::npos);
  EXPECT_EQ(f.find_last_of(i, 5), 5U);
  EXPECT_EQ(f.find_last_of(i, 6), 6U);
  EXPECT_EQ(f.find_last_of(a, 4), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(f.find_last_of(d), StringPiece::npos);
  EXPECT_EQ(f.find_last_of(e), StringPiece::npos);
  EXPECT_EQ(f.find_last_of(d, 4), StringPiece::npos);
  EXPECT_EQ(f.find_last_of(e, 4), StringPiece::npos);
  EXPECT_EQ(d.find_last_of(d), StringPiece::npos);
  EXPECT_EQ(d.find_last_of(e), StringPiece::npos);
  EXPECT_EQ(e.find_last_of(d), StringPiece::npos);
  EXPECT_EQ(e.find_last_of(e), StringPiece::npos);
  EXPECT_EQ(d.find_last_of(f), StringPiece::npos);
  EXPECT_EQ(e.find_last_of(f), StringPiece::npos);
  EXPECT_EQ(d.find_last_of(d, 4), StringPiece::npos);
  EXPECT_EQ(d.find_last_of(e, 4), StringPiece::npos);
  EXPECT_EQ(e.find_last_of(d, 4), StringPiece::npos);
  EXPECT_EQ(e.find_last_of(e, 4), StringPiece::npos);
  EXPECT_EQ(d.find_last_of(f, 4), StringPiece::npos);
  EXPECT_EQ(e.find_last_of(f, 4), StringPiece::npos);

  EXPECT_EQ(a.find_last_not_of(b), a.size()-1);
  EXPECT_EQ(a.find_last_not_of(c), 22U);
  EXPECT_EQ(b.find_last_not_of(a), StringPiece::npos);
  EXPECT_EQ(b.find_last_not_of(b), StringPiece::npos);
  EXPECT_EQ(f.find_last_not_of(i), 4U);
  EXPECT_EQ(a.find_last_not_of(c, 24), 22U);
  EXPECT_EQ(a.find_last_not_of(b, 3), 3U);
  EXPECT_EQ(a.find_last_not_of(b, 2), StringPiece::npos);
  // empty string nonsense
  EXPECT_EQ(f.find_last_not_of(d), f.size()-1);
  EXPECT_EQ(f.find_last_not_of(e), f.size()-1);
  EXPECT_EQ(f.find_last_not_of(d, 4), 4U);
  EXPECT_EQ(f.find_last_not_of(e, 4), 4U);
  EXPECT_EQ(d.find_last_not_of(d), StringPiece::npos);
  EXPECT_EQ(d.find_last_not_of(e), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of(d), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of(e), StringPiece::npos);
  EXPECT_EQ(d.find_last_not_of(f), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of(f), StringPiece::npos);
  EXPECT_EQ(d.find_last_not_of(d, 4), StringPiece::npos);
  EXPECT_EQ(d.find_last_not_of(e, 4), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of(d, 4), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of(e, 4), StringPiece::npos);
  EXPECT_EQ(d.find_last_not_of(f, 4), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of(f, 4), StringPiece::npos);

  EXPECT_EQ(h.find_last_not_of('x'), h.size() - 1);
  EXPECT_EQ(h.find_last_not_of('='), StringPiece::npos);
  EXPECT_EQ(b.find_last_not_of('c'), 1U);
  EXPECT_EQ(h.find_last_not_of('x', 2), 2U);
  EXPECT_EQ(h.find_last_not_of('=', 2), StringPiece::npos);
  EXPECT_EQ(b.find_last_not_of('b', 1), 0U);
  // empty string nonsense
  EXPECT_EQ(d.find_last_not_of('x'), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of('x'), StringPiece::npos);
  EXPECT_EQ(d.find_last_not_of('\0'), StringPiece::npos);
  EXPECT_EQ(e.find_last_not_of('\0'), StringPiece::npos);
}

// Continued from STL2
TEST(StringViewTest, STL2Substr) {
  const StringPiece a("abcdefghijklmnopqrstuvwxyz");
  const StringPiece b("abc");
  const StringPiece c("xyz");
  StringPiece d("foobar");
  const StringPiece e;

  d = StringPiece();
  EXPECT_EQ(a.substr(0, 3), b);
  EXPECT_EQ(a.substr(23), c);
  EXPECT_EQ(a.substr(23, 3), c);
  EXPECT_EQ(a.substr(23, 99), c);
  EXPECT_EQ(a.substr(0), a);
  EXPECT_EQ(a.substr(), a);
  EXPECT_EQ(a.substr(3, 2), "de");
  // empty string nonsense
  EXPECT_EQ(d.substr(0, 99), e);
  // use of npos
  EXPECT_EQ(a.substr(0, StringPiece::npos), a);
  EXPECT_EQ(a.substr(23, StringPiece::npos), c);
  // throw exception
#ifdef ABSL_HAVE_EXCEPTIONS
  EXPECT_THROW((void)a.substr(99, 2), std::out_of_range);
#endif
}

TEST(StringViewTest, UTF8) {
  std::string utf8 = "\u00E1";
  std::string utf8_twice = utf8 + " " + utf8;
  size_t utf8_len = strlen(utf8.data());
  EXPECT_EQ(utf8_len, StringPiece(utf8_twice).find_first_of(" "));
  EXPECT_EQ(utf8_len, StringPiece(utf8_twice).find_first_of(" \t"));
}

TEST(StringViewTest, FindConformance) {
  struct {
    std::string haystack;
    std::string needle;
  } specs[] = {
    {"", ""},
    {"", "a"},
    {"a", ""},
    {"a", "a"},
    {"a", "b"},
    {"aa", ""},
    {"aa", "a"},
    {"aa", "b"},
    {"ab", "a"},
    {"ab", "b"},
    {"abcd", ""},
    {"abcd", "a"},
    {"abcd", "d"},
    {"abcd", "ab"},
    {"abcd", "bc"},
    {"abcd", "cd"},
    {"abcd", "abcd"},
  };
  for (const auto& s : specs) {
    SCOPED_TRACE(s.haystack);
    SCOPED_TRACE(s.needle);
    std::string st = s.haystack;
    StringPiece sp = s.haystack;
    for (size_t i = 0; i <= sp.size(); ++i) {
      size_t pos = (i == sp.size()) ? StringPiece::npos : i;
      SCOPED_TRACE(pos);
      EXPECT_EQ(sp.find(s.needle, pos),
                st.find(s.needle, pos));
      EXPECT_EQ(sp.rfind(s.needle, pos),
                st.rfind(s.needle, pos));
      EXPECT_EQ(sp.find_first_of(s.needle, pos),
                st.find_first_of(s.needle, pos));
      EXPECT_EQ(sp.find_first_not_of(s.needle, pos),
                st.find_first_not_of(s.needle, pos));
      EXPECT_EQ(sp.find_last_of(s.needle, pos),
                st.find_last_of(s.needle, pos));
      EXPECT_EQ(sp.find_last_not_of(s.needle, pos),
                st.find_last_not_of(s.needle, pos));
    }
  }
}

TEST(StringViewTest, Remove) {
  StringPiece a("foobar");
  std::string s1("123");
  s1 += '\0';
  s1 += "456";
  StringPiece e;
  std::string s2;

  // remove_prefix
  StringPiece c(a);
  c.remove_prefix(3);
  EXPECT_EQ(c, "bar");
  c = a;
  c.remove_prefix(0);
  EXPECT_EQ(c, a);
  c.remove_prefix(c.size());
  EXPECT_EQ(c, e);

  // remove_suffix
  c = a;
  c.remove_suffix(3);
  EXPECT_EQ(c, "foo");
  c = a;
  c.remove_suffix(0);
  EXPECT_EQ(c, a);
  c.remove_suffix(c.size());
  EXPECT_EQ(c, e);
}

TEST(StringViewTest, Set) {
  StringPiece a("foobar");
  StringPiece empty;
  StringPiece b;

  // set
  b = StringPiece("foobar", 6);
  EXPECT_EQ(b, a);
  b = StringPiece("foobar", 0);
  EXPECT_EQ(b, empty);
  b = StringPiece("foobar", 7);
  EXPECT_NE(b, a);

  b = StringPiece("foobar");
  EXPECT_EQ(b, a);
}

TEST(StringViewTest, FrontBack) {
  static const char arr[] = "abcd";
  const StringPiece csp(arr, 4);
  EXPECT_EQ(&arr[0], &csp.front());
  EXPECT_EQ(&arr[3], &csp.back());
}

TEST(StringViewTest, FrontBackSingleChar) {
  static const char c = 'a';
  const StringPiece csp(&c, 1);
  EXPECT_EQ(&c, &csp.front());
  EXPECT_EQ(&c, &csp.back());
}

#if !defined(oak_HAVE_STD_StringPiece)

TEST(StringViewTest, NULLInput) {
  StringPiece s;
  EXPECT_EQ(s.data(), nullptr);
  EXPECT_EQ(s.size(), 0U);

  s = StringPiece(nullptr);
  EXPECT_EQ(s.data(), nullptr);
  EXPECT_EQ(s.size(), 0U);

  // .ToString() on a StringPiece with nullptr should produce the empty
  // string.
  EXPECT_EQ("", std::string(s));
}
#endif

TEST(StringViewTest, Comparisons2) {
  // The `compare` member has 6 overloads (v: StringPiece, s: const char*):
  //  (1) compare(v)
  //  (2) compare(pos1, count1, v)
  //  (3) compare(pos1, count1, v, pos2, count2)
  //  (4) compare(s)
  //  (5) compare(pos1, count1, s)
  //  (6) compare(pos1, count1, s, count2)

  StringPiece abc("abcdefghijklmnopqrstuvwxyz");

  // check comparison operations on strings longer than 4 bytes.
  EXPECT_EQ(abc, StringPiece("abcdefghijklmnopqrstuvwxyz"));
  EXPECT_EQ(abc.compare(StringPiece("abcdefghijklmnopqrstuvwxyz")), 0);

  EXPECT_LT(abc, StringPiece("abcdefghijklmnopqrstuvwxzz"));
  EXPECT_LT(abc.compare(StringPiece("abcdefghijklmnopqrstuvwxzz")), 0);

  EXPECT_GT(abc, StringPiece("abcdefghijklmnopqrstuvwxyy"));
  EXPECT_GT(abc.compare(StringPiece("abcdefghijklmnopqrstuvwxyy")), 0);

  // The "substr" variants of `compare`.
  StringPiece digits("0123456789");
  auto npos = StringPiece::npos;

  // Taking StringPiece
  EXPECT_EQ(digits.compare(3, npos, StringPiece("3456789")), 0);  // 2
  EXPECT_EQ(digits.compare(3, 4, StringPiece("3456")), 0);        // 2
  EXPECT_EQ(digits.compare(10, 0, StringPiece()), 0);             // 2
  EXPECT_EQ(digits.compare(3, 4, StringPiece("0123456789"), 3, 4),
            0);  // 3
  EXPECT_LT(digits.compare(3, 4, StringPiece("0123456789"), 3, 5),
            0);  // 3
  EXPECT_LT(digits.compare(0, npos, StringPiece("0123456789"), 3, 5),
            0);  // 3
  // Taking const char*
  EXPECT_EQ(digits.compare(3, 4, "3456"), 0);                 // 5
  EXPECT_EQ(digits.compare(3, npos, "3456789"), 0);           // 5
  EXPECT_EQ(digits.compare(10, 0, ""), 0);                    // 5
  EXPECT_EQ(digits.compare(3, 4, "0123456789", 3, 4), 0);     // 6
  EXPECT_LT(digits.compare(3, 4, "0123456789", 3, 5), 0);     // 6
  EXPECT_LT(digits.compare(0, npos, "0123456789", 3, 5), 0);  // 6
}

TEST(StringViewTest, At) {
  StringPiece abc = "abc";
  EXPECT_EQ(abc.at(0), 'a');
  EXPECT_EQ(abc.at(1), 'b');
  EXPECT_EQ(abc.at(2), 'c');
}

TEST(StringViewTest, StartsWith) {
  StringPiece v = "abc123";
  EXPECT_EQ(v.starts_with('a'), true);
  EXPECT_EQ(v.starts_with('b'), false);
  EXPECT_EQ(v.starts_with("abc"), true);
  EXPECT_EQ(v.starts_with("abd"), false);
  EXPECT_EQ(v.starts_with("abd", 2), true);
  EXPECT_EQ(v.starts_with("abd", 3), false);
}

TEST(StringViewTest, EndsWith) {
  StringPiece v = "abc123";
  EXPECT_EQ(v.ends_with('3'), true);
  EXPECT_EQ(v.ends_with('2'), false);
  EXPECT_EQ(v.ends_with("123"), true);
  EXPECT_EQ(v.ends_with("124"), false);
  EXPECT_EQ(v.ends_with("234", 2), true);
  EXPECT_EQ(v.ends_with("234", 3), false);
}

TEST(StringViewTest, ExplicitConversionOperator) {
  StringPiece sp = "hi";
  EXPECT_EQ(sp, std::string(sp));
}

TEST(StringViewTest, ConstexprCompiles) {
  constexpr StringPiece sp;
  constexpr StringPiece cstr_len("cstr", 4);

  constexpr StringPiece::iterator const_begin = cstr_len.begin();
  constexpr StringPiece::iterator const_end = cstr_len.end();
  constexpr StringPiece::size_type const_size = cstr_len.size();
  constexpr StringPiece::size_type const_length = cstr_len.length();
  static_assert(const_begin + const_size == const_end,
                "pointer arithmetic check");
  static_assert(const_begin + const_length == const_end,
                "pointer arithmetic check");
  EXPECT_EQ(const_begin + const_size, const_end);
  EXPECT_EQ(const_begin + const_length, const_end);

  constexpr bool isempty = sp.empty();
  EXPECT_TRUE(isempty);

  constexpr const char c = cstr_len[2];
  EXPECT_EQ(c, 't');

  constexpr const char cfront = cstr_len.front();
  constexpr const char cback = cstr_len.back();
  EXPECT_EQ(cfront, 'c');
  EXPECT_EQ(cback, 'r');

  constexpr const char* np = sp.data();
  constexpr const char* cstr_ptr = cstr_len.data();
  EXPECT_EQ(np, nullptr);
  EXPECT_NE(cstr_ptr, nullptr);

  constexpr size_t sp_npos = sp.npos;
  EXPECT_EQ(sp_npos, static_cast<size_t>(-1));
}

TEST(StringViewTest, ConstexprMethods) {
  // substr
  constexpr StringPiece foobar("foobar", 6);
  constexpr StringPiece foo = foobar.substr(0, 3);
  constexpr StringPiece bar = foobar.substr(3);
  EXPECT_EQ(foo, "foo");
  EXPECT_EQ(bar, "bar");
}

TEST(StringViewTest, Noexcept) {
  EXPECT_TRUE((std::is_nothrow_constructible<StringPiece,
                                             const std::string&>::value));
  EXPECT_TRUE((std::is_nothrow_constructible<StringPiece,
                                             const std::string&>::value));
  EXPECT_TRUE(std::is_nothrow_constructible<StringPiece>::value);
  constexpr StringPiece sp;
  EXPECT_TRUE(noexcept(sp.begin()));
  EXPECT_TRUE(noexcept(sp.end()));
  EXPECT_TRUE(noexcept(sp.cbegin()));
  EXPECT_TRUE(noexcept(sp.cend()));
  EXPECT_TRUE(noexcept(sp.rbegin()));
  EXPECT_TRUE(noexcept(sp.rend()));
  EXPECT_TRUE(noexcept(sp.crbegin()));
  EXPECT_TRUE(noexcept(sp.crend()));
  EXPECT_TRUE(noexcept(sp.size()));
  EXPECT_TRUE(noexcept(sp.length()));
  EXPECT_TRUE(noexcept(sp.empty()));
  EXPECT_TRUE(noexcept(sp.data()));
  EXPECT_TRUE(noexcept(sp.compare(sp)));
  EXPECT_TRUE(noexcept(sp.find(sp)));
  EXPECT_TRUE(noexcept(sp.find('f')));
  EXPECT_TRUE(noexcept(sp.rfind(sp)));
  EXPECT_TRUE(noexcept(sp.rfind('f')));
  EXPECT_TRUE(noexcept(sp.find_first_of(sp)));
  EXPECT_TRUE(noexcept(sp.find_first_of('f')));
  EXPECT_TRUE(noexcept(sp.find_last_of(sp)));
  EXPECT_TRUE(noexcept(sp.find_last_of('f')));
  EXPECT_TRUE(noexcept(sp.find_first_not_of(sp)));
  EXPECT_TRUE(noexcept(sp.find_first_not_of('f')));
  EXPECT_TRUE(noexcept(sp.find_last_not_of(sp)));
  EXPECT_TRUE(noexcept(sp.find_last_not_of('f')));
}

TEST(ComparisonOpsTest, StringCompareNotAmbiguous) {
  EXPECT_EQ("hello", std::string("hello"));
  EXPECT_LT("hello", std::string("world"));
}

TEST(ComparisonOpsTest, HeterogenousStringViewEquals) {
  EXPECT_EQ(StringPiece("hello"), std::string("hello"));
  EXPECT_EQ("hello", StringPiece("hello"));
}

TEST(FindOneCharTest, EdgeCases) {
  StringPiece a("xxyyyxx");

  // Set a = "xyyyx".
  a.remove_prefix(1);
  a.remove_suffix(1);

  EXPECT_EQ(0U, a.find('x'));
  EXPECT_EQ(0U, a.find('x', 0));
  EXPECT_EQ(4U, a.find('x', 1));
  EXPECT_EQ(4U, a.find('x', 4));
  EXPECT_EQ(StringPiece::npos, a.find('x', 5));

  EXPECT_EQ(4U, a.rfind('x'));
  EXPECT_EQ(4U, a.rfind('x', 5));
  EXPECT_EQ(4U, a.rfind('x', 4));
  EXPECT_EQ(0U, a.rfind('x', 3));
  EXPECT_EQ(0U, a.rfind('x', 0));

  // Set a = "yyy".
  a.remove_prefix(1);
  a.remove_suffix(1);

  EXPECT_EQ(StringPiece::npos, a.find('x'));
  EXPECT_EQ(StringPiece::npos, a.rfind('x'));
}

class StringViewStreamTest : public ::testing::Test {
 public:
  // Set negative 'width' for right justification.
  template <typename T>
  std::string Pad(const T& s, int width, char fill = 0) {
    std::ostringstream oss;
    if (fill != 0) {
      oss << std::setfill(fill);
    }
    if (width < 0) {
      width = -width;
      oss << std::right;
    }
    oss << std::setw(width) << s;
    return oss.str();
  }
};

TEST_F(StringViewStreamTest, Padding) {
  std::string s("hello");
  StringPiece sp(s);
  for (int w = -64; w < 64; ++w) {
    SCOPED_TRACE(w);
    EXPECT_EQ(Pad(s, w), Pad(sp, w));
  }
  for (int w = -64; w < 64; ++w) {
    SCOPED_TRACE(w);
    EXPECT_EQ(Pad(s, w, '#'), Pad(sp, w, '#'));
  }
}

TEST_F(StringViewStreamTest, ResetsWidth) {
  // Width should reset after one formatted write.
  // If we weren't resetting width after formatting the StringPiece,
  // we'd have width=5 carrying over to the printing of the "]",
  // creating "[###hi####]".
  std::string s = "hi";
  StringPiece sp = s;
  {
    std::ostringstream oss;
    oss << "[" << std::setfill('#') << std::setw(5) << s << "]";
    ASSERT_EQ("[###hi]", oss.str());
  }
  {
    std::ostringstream oss;
    oss << "[" << std::setfill('#') << std::setw(5) << sp << "]";
    EXPECT_EQ("[###hi]", oss.str());
  }
}

TEST(StringViewHashTest, Hash) {
  std::string s("hello");
  std::string empty;
  EXPECT_EQ(std::hash<StringPiece>{}(s), std::hash<std::string>{}(s));
  EXPECT_EQ(std::hash<StringPiece>{}(empty), 0U);
}

}  // anonymous namespace
