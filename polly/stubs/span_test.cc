#include "polly/stubs/span.h"

#include <array>
#include <initializer_list>
#include <numeric>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace {

MATCHER_P(DataIs, data,
          std::string(negation ? "data() isn't " : "data() is ") +
              testing::PrintToString(data)) {
  return arg.data() == data;
}

template <typename T>
auto SpanIs(T data, size_t size)
    -> decltype(testing::AllOf(DataIs(data), testing::SizeIs(size))) {
  return testing::AllOf(DataIs(data), testing::SizeIs(size));
}

template <typename Container>
auto SpanIs(const Container& c) -> decltype(SpanIs(c.data(), c.size())) {
  return SpanIs(c.data(), c.size());
}

std::vector<int> MakeRamp(int len, int offset = 0) {
  std::vector<int> v(len);
  std::iota(v.begin(), v.end(), offset);
  return v;
}

TEST(IntSpan, EmptyCtors) {
  polly::Span<int> s;
  EXPECT_THAT(s, SpanIs(nullptr, 0));
}

TEST(IntSpan, PtrLenCtor) {
  int a[] = {1, 2, 3};
  polly::Span<int> s(&a[0], 2);
  EXPECT_THAT(s, SpanIs(a, 2));
}

TEST(IntSpan, ArrayCtor) {
  int a[] = {1, 2, 3};
  polly::Span<int> s(a);
  EXPECT_THAT(s, SpanIs(a, 3));

  EXPECT_TRUE((std::is_constructible<polly::Span<const int>, int[3]>::value));
  EXPECT_TRUE(
      (std::is_constructible<polly::Span<const int>, const int[3]>::value));
  EXPECT_FALSE((std::is_constructible<polly::Span<int>, const int[3]>::value));
  EXPECT_TRUE((std::is_convertible<int[3], polly::Span<const int>>::value));
  EXPECT_TRUE(
      (std::is_convertible<const int[3], polly::Span<const int>>::value));
}

template <typename T>
void TakesGenericSpan(polly::Span<T>) {}

TEST(IntSpan, ContainerCtor) {
  std::vector<int> empty;
  polly::Span<int> s_empty(empty);
  EXPECT_THAT(s_empty, SpanIs(empty));

  std::vector<int> filled{1, 2, 3};
  polly::Span<int> s_filled(filled);
  EXPECT_THAT(s_filled, SpanIs(filled));

  polly::Span<int> s_from_span(filled);
  EXPECT_THAT(s_from_span, SpanIs(s_filled));

  polly::Span<const int> const_filled = filled;
  EXPECT_THAT(const_filled, SpanIs(filled));

  polly::Span<const int> const_from_span = s_filled;
  EXPECT_THAT(const_from_span, SpanIs(s_filled));

  EXPECT_TRUE(
      (std::is_convertible<std::vector<int>&, polly::Span<const int>>::value));
  EXPECT_TRUE(
      (std::is_convertible<polly::Span<int>&, polly::Span<const int>>::value));

  TakesGenericSpan(polly::Span<int>(filled));
}

// A struct supplying shallow data() const.
struct ContainerWithShallowConstData {
  std::vector<int> storage;
  int* data() const { return const_cast<int*>(storage.data()); }
  int size() const { return storage.size(); }
};

TEST(IntSpan, ShallowConstness) {
  const ContainerWithShallowConstData c{MakeRamp(20)};
  polly::Span<int> s(
      c);  // We should be able to do this even though data() is const.
  s[0] = -1;
  EXPECT_EQ(c.storage[0], -1);
}

TEST(CharSpan, StringCtor) {
  std::string empty = "";
  polly::Span<char> s_empty(empty);
  EXPECT_THAT(s_empty, SpanIs(empty));

  std::string abc = "abc";
  polly::Span<char> s_abc(abc);
  EXPECT_THAT(s_abc, SpanIs(abc));

  polly::Span<const char> s_const_abc = abc;
  EXPECT_THAT(s_const_abc, SpanIs(abc));

  EXPECT_FALSE((std::is_constructible<polly::Span<int>, std::string>::value));
  EXPECT_FALSE(
      (std::is_constructible<polly::Span<const int>, std::string>::value));
  EXPECT_TRUE(
      (std::is_convertible<std::string, polly::Span<const char>>::value));
}

TEST(IntSpan, FromConstPointer) {
  EXPECT_TRUE((std::is_constructible<polly::Span<const int* const>,
                                     std::vector<int*>>::value));
  EXPECT_TRUE((std::is_constructible<polly::Span<const int* const>,
                                     std::vector<const int*>>::value));
  EXPECT_FALSE((
      std::is_constructible<polly::Span<const int*>, std::vector<int*>>::value));
  EXPECT_FALSE((
      std::is_constructible<polly::Span<int*>, std::vector<const int*>>::value));
}

struct TypeWithMisleadingData {
  int& data() { return i; }
  int size() { return 1; }
  int i;
};

struct TypeWithMisleadingSize {
  int* data() { return &i; }
  const char* size() { return "1"; }
  int i;
};

TEST(IntSpan, EvilTypes) {
  EXPECT_FALSE(
      (std::is_constructible<polly::Span<int>, TypeWithMisleadingData&>::value));
  EXPECT_FALSE(
      (std::is_constructible<polly::Span<int>, TypeWithMisleadingSize&>::value));
}

struct Base {
  int* data() { return &i; }
  int size() { return 1; }
  int i;
};
struct Derived : Base {};

TEST(IntSpan, SpanOfDerived) {
  EXPECT_TRUE((std::is_constructible<polly::Span<int>, Base&>::value));
  EXPECT_TRUE((std::is_constructible<polly::Span<int>, Derived&>::value));
  EXPECT_FALSE(
      (std::is_constructible<polly::Span<Base>, std::vector<Derived>>::value));
}

void TestInitializerList(polly::Span<const int> s, const std::vector<int>& v) {
  EXPECT_TRUE(std::equal(s.begin(), s.end(), v.begin()));
}

TEST(ConstIntSpan, InitializerListConversion) {
  TestInitializerList({}, {});
  TestInitializerList({1}, {1});
  TestInitializerList({1, 2, 3}, {1, 2, 3});

  EXPECT_FALSE((std::is_constructible<polly::Span<int>,
                                      std::initializer_list<int>>::value));
  EXPECT_FALSE((
      std::is_convertible<polly::Span<int>, std::initializer_list<int>>::value));
}

TEST(IntSpan, Data) {
  int i;
  polly::Span<int> s(&i, 1);
  EXPECT_EQ(&i, s.data());
}

TEST(IntSpan, SizeLengthEmpty) {
  polly::Span<int> empty;
  EXPECT_EQ(empty.size(), 0);
  EXPECT_TRUE(empty.empty());
  EXPECT_EQ(empty.size(), empty.length());

  auto v = MakeRamp(10);
  polly::Span<int> s(v);
  EXPECT_EQ(s.size(), 10);
  EXPECT_FALSE(s.empty());
  EXPECT_EQ(s.size(), s.length());
}

TEST(IntSpan, ElementAccess) {
  auto v = MakeRamp(10);
  polly::Span<int> s(v);
  for (size_t i = 0; i < s.size(); ++i) {
    EXPECT_EQ(s[i], s.at(i));
  }

  EXPECT_EQ(s.front(), s[0]);
  EXPECT_EQ(s.back(), s[9]);
}

TEST(IntSpan, RemovePrefixAndSuffix) {
  auto v = MakeRamp(20, 1);
  polly::Span<int> s(v);
  EXPECT_EQ(s.size(), 20);

  s.remove_suffix(0);
  s.remove_prefix(0);
  EXPECT_EQ(s.size(), 20);

  s.remove_prefix(1);
  EXPECT_EQ(s.size(), 19);
  EXPECT_EQ(s[0], 2);

  s.remove_suffix(1);
  EXPECT_EQ(s.size(), 18);
  EXPECT_EQ(s.back(), 19);

  s.remove_prefix(7);
  EXPECT_EQ(s.size(), 11);
  EXPECT_EQ(s[0], 9);

  s.remove_suffix(11);
  EXPECT_EQ(s.size(), 0);

  EXPECT_EQ(v, MakeRamp(20, 1));
}

TEST(IntSpan, Subspan) {
  std::vector<int> empty;
  EXPECT_EQ(polly::MakeSpan(empty).subspan(), empty);
  EXPECT_THAT(polly::MakeSpan(empty).subspan(0, 0), SpanIs(empty));
  EXPECT_THAT(polly::MakeSpan(empty).subspan(0, polly::Span<const int>::npos),
              SpanIs(empty));

  auto ramp = MakeRamp(10);
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(), SpanIs(ramp));
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(0, 10), SpanIs(ramp));
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(0, polly::Span<const int>::npos),
              SpanIs(ramp));
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(0, 3), SpanIs(ramp.data(), 3));
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(5, polly::Span<const int>::npos),
              SpanIs(ramp.data() + 5, 5));
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(3, 3), SpanIs(ramp.data() + 3, 3));
  EXPECT_THAT(polly::MakeSpan(ramp).subspan(10, 5), SpanIs(ramp.data() + 10, 0));

#ifdef POLLY_HAVE_EXCEPTIONS
  EXPECT_THROW(polly::MakeSpan(ramp).subspan(11, 5), std::out_of_range);
#endif
}

TEST(IntSpan, First) {
  std::vector<int> empty;
  EXPECT_THAT(polly::MakeSpan(empty).first(0), SpanIs(empty));

  auto ramp = MakeRamp(10);
  EXPECT_THAT(polly::MakeSpan(ramp).first(0), SpanIs(ramp.data(), 0));
  EXPECT_THAT(polly::MakeSpan(ramp).first(10), SpanIs(ramp));
  EXPECT_THAT(polly::MakeSpan(ramp).first(3), SpanIs(ramp.data(), 3));

#ifdef POLLY_HAVE_EXCEPTIONS
  EXPECT_THROW(polly::MakeSpan(ramp).first(11), std::out_of_range);
#endif
}

TEST(IntSpan, Last) {
  std::vector<int> empty;
  EXPECT_THAT(polly::MakeSpan(empty).last(0), SpanIs(empty));

  auto ramp = MakeRamp(10);
  EXPECT_THAT(polly::MakeSpan(ramp).last(0), SpanIs(ramp.data() + 10, 0));
  EXPECT_THAT(polly::MakeSpan(ramp).last(10), SpanIs(ramp));
  EXPECT_THAT(polly::MakeSpan(ramp).last(3), SpanIs(ramp.data() + 7, 3));

#ifdef POLLY_HAVE_EXCEPTIONS
  EXPECT_THROW(polly::MakeSpan(ramp).last(11), std::out_of_range);
#endif
}

TEST(IntSpan, MakeSpanPtrLength) {
  std::vector<int> empty;
  auto s_empty = polly::MakeSpan(empty.data(), empty.size());
  EXPECT_THAT(s_empty, SpanIs(empty));

  std::array<int, 3> a{{1, 2, 3}};
  auto s = polly::MakeSpan(a.data(), a.size());
  EXPECT_THAT(s, SpanIs(a));

  EXPECT_THAT(polly::MakeConstSpan(empty.data(), empty.size()), SpanIs(s_empty));
  EXPECT_THAT(polly::MakeConstSpan(a.data(), a.size()), SpanIs(s));
}

TEST(IntSpan, MakeSpanTwoPtrs) {
  std::vector<int> empty;
  auto s_empty = polly::MakeSpan(empty.data(), empty.data());
  EXPECT_THAT(s_empty, SpanIs(empty));

  std::vector<int> v{1, 2, 3};
  auto s = polly::MakeSpan(v.data(), v.data() + 1);
  EXPECT_THAT(s, SpanIs(v.data(), 1));

  EXPECT_THAT(polly::MakeConstSpan(empty.data(), empty.data()), SpanIs(s_empty));
  EXPECT_THAT(polly::MakeConstSpan(v.data(), v.data() + 1), SpanIs(s));
}

TEST(IntSpan, MakeSpanContainer) {
  std::vector<int> empty;
  auto s_empty = polly::MakeSpan(empty);
  EXPECT_THAT(s_empty, SpanIs(empty));

  std::vector<int> v{1, 2, 3};
  auto s = polly::MakeSpan(v);
  EXPECT_THAT(s, SpanIs(v));

  EXPECT_THAT(polly::MakeConstSpan(empty), SpanIs(s_empty));
  EXPECT_THAT(polly::MakeConstSpan(v), SpanIs(s));

  EXPECT_THAT(polly::MakeSpan(s), SpanIs(s));
  EXPECT_THAT(polly::MakeConstSpan(s), SpanIs(s));
}

TEST(CharSpan, MakeSpanString) {
  std::string empty = "";
  auto s_empty = polly::MakeSpan(empty);
  EXPECT_THAT(s_empty, SpanIs(empty));

  std::string str = "abc";
  auto s_str = polly::MakeSpan(str);
  EXPECT_THAT(s_str, SpanIs(str));

  EXPECT_THAT(polly::MakeConstSpan(empty), SpanIs(s_empty));
  EXPECT_THAT(polly::MakeConstSpan(str), SpanIs(s_str));
}

TEST(IntSpan, MakeSpanArray) {
  int a[] = {1, 2, 3};
  auto s = polly::MakeSpan(a);
  EXPECT_THAT(s, SpanIs(a, 3));

  const int ca[] = {1, 2, 3};
  auto s_ca = polly::MakeSpan(ca);
  EXPECT_THAT(s_ca, SpanIs(ca, 3));

  EXPECT_THAT(polly::MakeConstSpan(a), SpanIs(s));
  EXPECT_THAT(polly::MakeConstSpan(ca), SpanIs(s_ca));
}

// Compile-asserts that the argument has the expected decayed type.
template <typename Expected, typename T>
void CheckType(const T& /* value */) {
  testing::StaticAssertTypeEq<Expected, T>();
}

TEST(IntSpan, MakeSpanTypes) {
  std::vector<int> vec;
  const std::vector<int> cvec;
  int a[1];
  const int ca[] = {1};
  int* ip = a;
  const int* cip = ca;
  std::string s = "";
  const std::string cs = "";
  CheckType<polly::Span<int>>(polly::MakeSpan(vec));
  CheckType<polly::Span<const int>>(polly::MakeSpan(cvec));
  CheckType<polly::Span<int>>(polly::MakeSpan(ip, ip + 1));
  CheckType<polly::Span<int>>(polly::MakeSpan(ip, 1));
  CheckType<polly::Span<const int>>(polly::MakeSpan(cip, cip + 1));
  CheckType<polly::Span<const int>>(polly::MakeSpan(cip, 1));
  CheckType<polly::Span<int>>(polly::MakeSpan(a));
  CheckType<polly::Span<int>>(polly::MakeSpan(a, a + 1));
  CheckType<polly::Span<int>>(polly::MakeSpan(a, 1));
  CheckType<polly::Span<const int>>(polly::MakeSpan(ca));
  CheckType<polly::Span<const int>>(polly::MakeSpan(ca, ca + 1));
  CheckType<polly::Span<const int>>(polly::MakeSpan(ca, 1));
  CheckType<polly::Span<char>>(polly::MakeSpan(s));
  CheckType<polly::Span<const char>>(polly::MakeSpan(cs));
}

TEST(ConstIntSpan, MakeConstSpanTypes) {
  std::vector<int> vec;
  const std::vector<int> cvec;
  int array[1];
  const int carray[] = {0};
  int* ptr = array;
  const int* cptr = carray;
  std::string s = "";
  std::string cs = "";
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(vec));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(cvec));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(ptr, ptr + 1));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(ptr, 1));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(cptr, cptr + 1));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(cptr, 1));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(array));
  CheckType<polly::Span<const int>>(polly::MakeConstSpan(carray));
  CheckType<polly::Span<const char>>(polly::MakeConstSpan(s));
  CheckType<polly::Span<const char>>(polly::MakeConstSpan(cs));
}

TEST(IntSpan, Equality) {
  const int arr1[] = {1, 2, 3, 4, 5};
  int arr2[] = {1, 2, 3, 4, 5};
  std::vector<int> vec1(std::begin(arr1), std::end(arr1));
  std::vector<int> vec2 = vec1;
  std::vector<int> other_vec = {2, 4, 6, 8, 10};
  // These two slices are from different vectors, but have the same size and
  // have the same elements (right now).  They should compare equal. Test both
  // == and !=.
  const polly::Span<const int> from1 = vec1;
  const polly::Span<const int> from2 = vec2;
  EXPECT_EQ(from1, from1);
  EXPECT_FALSE(from1 != from1);
  EXPECT_EQ(from1, from2);
  EXPECT_FALSE(from1 != from2);

  // These two slices have different underlying vector values. They should be
  // considered not equal. Test both == and !=.
  const polly::Span<const int> from_other = other_vec;
  EXPECT_NE(from1, from_other);
  EXPECT_FALSE(from1 == from_other);

  // Comparison between a vector and its slice should be equal. And vice-versa.
  // This ensures implicit conversion to Span works on both sides of ==.
  EXPECT_EQ(vec1, from1);
  EXPECT_FALSE(vec1 != from1);
  EXPECT_EQ(from1, vec1);
  EXPECT_FALSE(from1 != vec1);

  // This verifies that polly::Span<T> can be compared freely with
  // polly::Span<const T>.
  const polly::Span<int> mutable_from1(vec1);
  const polly::Span<int> mutable_from2(vec2);
  EXPECT_EQ(from1, mutable_from1);
  EXPECT_EQ(mutable_from1, from1);
  EXPECT_EQ(mutable_from1, mutable_from2);
  EXPECT_EQ(mutable_from2, mutable_from1);

  // Comparison between a vector and its slice should be equal for mutable
  // Spans as well.
  EXPECT_EQ(vec1, mutable_from1);
  EXPECT_FALSE(vec1 != mutable_from1);
  EXPECT_EQ(mutable_from1, vec1);
  EXPECT_FALSE(mutable_from1 != vec1);

  // Comparison between convertible-to-Span-of-const and Span-of-mutable. Arrays
  // are used because they're the only value type which converts to a
  // Span-of-mutable. EXPECT_TRUE is used instead of EXPECT_EQ to avoid
  // array-to-pointer decay.
  EXPECT_TRUE(arr1 == mutable_from1);
  EXPECT_FALSE(arr1 != mutable_from1);
  EXPECT_TRUE(mutable_from1 == arr1);
  EXPECT_FALSE(mutable_from1 != arr1);

  // Comparison between convertible-to-Span-of-mutable and Span-of-const
  EXPECT_TRUE(arr2 == from1);
  EXPECT_FALSE(arr2 != from1);
  EXPECT_TRUE(from1 == arr2);
  EXPECT_FALSE(from1 != arr2);

  // With a different size, the array slices should not be equal.
  EXPECT_NE(from1, polly::Span<const int>(from1).subspan(0, from1.size() - 1));

  // With different contents, the array slices should not be equal.
  ++vec2.back();
  EXPECT_NE(from1, from2);
}

class IntSpanOrderComparisonTest : public testing::Test {
 public:
  IntSpanOrderComparisonTest()
      : arr_before_{1, 2, 3},
        arr_after_{1, 2, 4},
        carr_after_{1, 2, 4},
        vec_before_(std::begin(arr_before_), std::end(arr_before_)),
        vec_after_(std::begin(arr_after_), std::end(arr_after_)),
        before_(vec_before_),
        after_(vec_after_),
        cbefore_(vec_before_),
        cafter_(vec_after_) {}

 protected:
  int arr_before_[3], arr_after_[3];
  const int carr_after_[3];
  std::vector<int> vec_before_, vec_after_;
  polly::Span<int> before_, after_;
  polly::Span<const int> cbefore_, cafter_;
};

TEST_F(IntSpanOrderComparisonTest, CompareSpans) {
  EXPECT_TRUE(cbefore_ < cafter_);
  EXPECT_TRUE(cbefore_ <= cafter_);
  EXPECT_TRUE(cafter_ > cbefore_);
  EXPECT_TRUE(cafter_ >= cbefore_);

  EXPECT_FALSE(cbefore_ > cafter_);
  EXPECT_FALSE(cafter_ < cbefore_);

  EXPECT_TRUE(before_ < after_);
  EXPECT_TRUE(before_ <= after_);
  EXPECT_TRUE(after_ > before_);
  EXPECT_TRUE(after_ >= before_);

  EXPECT_FALSE(before_ > after_);
  EXPECT_FALSE(after_ < before_);

  EXPECT_TRUE(cbefore_ < after_);
  EXPECT_TRUE(cbefore_ <= after_);
  EXPECT_TRUE(after_ > cbefore_);
  EXPECT_TRUE(after_ >= cbefore_);

  EXPECT_FALSE(cbefore_ > after_);
  EXPECT_FALSE(after_ < cbefore_);
}

TEST_F(IntSpanOrderComparisonTest, SpanOfConstAndContainer) {
  EXPECT_TRUE(cbefore_ < vec_after_);
  EXPECT_TRUE(cbefore_ <= vec_after_);
  EXPECT_TRUE(vec_after_ > cbefore_);
  EXPECT_TRUE(vec_after_ >= cbefore_);

  EXPECT_FALSE(cbefore_ > vec_after_);
  EXPECT_FALSE(vec_after_ < cbefore_);

  EXPECT_TRUE(arr_before_ < cafter_);
  EXPECT_TRUE(arr_before_ <= cafter_);
  EXPECT_TRUE(cafter_ > arr_before_);
  EXPECT_TRUE(cafter_ >= arr_before_);

  EXPECT_FALSE(arr_before_ > cafter_);
  EXPECT_FALSE(cafter_ < arr_before_);
}

TEST_F(IntSpanOrderComparisonTest, SpanOfMutableAndContainer) {
  EXPECT_TRUE(vec_before_ < after_);
  EXPECT_TRUE(vec_before_ <= after_);
  EXPECT_TRUE(after_ > vec_before_);
  EXPECT_TRUE(after_ >= vec_before_);

  EXPECT_FALSE(vec_before_ > after_);
  EXPECT_FALSE(after_ < vec_before_);

  EXPECT_TRUE(before_ < carr_after_);
  EXPECT_TRUE(before_ <= carr_after_);
  EXPECT_TRUE(carr_after_ > before_);
  EXPECT_TRUE(carr_after_ >= before_);

  EXPECT_FALSE(before_ > carr_after_);
  EXPECT_FALSE(carr_after_ < before_);
}

TEST_F(IntSpanOrderComparisonTest, EqualSpans) {
  EXPECT_FALSE(before_ < before_);
  EXPECT_TRUE(before_ <= before_);
  EXPECT_FALSE(before_ > before_);
  EXPECT_TRUE(before_ >= before_);
}

TEST_F(IntSpanOrderComparisonTest, Subspans) {
  auto subspan = before_.subspan(0, 1);
  EXPECT_TRUE(subspan < before_);
  EXPECT_TRUE(subspan <= before_);
  EXPECT_TRUE(before_ > subspan);
  EXPECT_TRUE(before_ >= subspan);

  EXPECT_FALSE(subspan > before_);
  EXPECT_FALSE(before_ < subspan);
}

TEST_F(IntSpanOrderComparisonTest, EmptySpans) {
  polly::Span<int> empty;
  EXPECT_FALSE(empty < empty);
  EXPECT_TRUE(empty <= empty);
  EXPECT_FALSE(empty > empty);
  EXPECT_TRUE(empty >= empty);

  EXPECT_TRUE(empty < before_);
  EXPECT_TRUE(empty <= before_);
  EXPECT_TRUE(before_ > empty);
  EXPECT_TRUE(before_ >= empty);

  EXPECT_FALSE(empty > before_);
  EXPECT_FALSE(before_ < empty);
}

TEST(IntSpan, ExposesContainerTypesAndConsts) {
  polly::Span<int> slice;
  CheckType<polly::Span<int>::iterator>(slice.begin());
  EXPECT_TRUE((std::is_convertible<decltype(slice.begin()),
                                   polly::Span<int>::const_iterator>::value));
  CheckType<polly::Span<int>::const_iterator>(slice.cbegin());
  EXPECT_TRUE((std::is_convertible<decltype(slice.end()),
                                   polly::Span<int>::const_iterator>::value));
  CheckType<polly::Span<int>::const_iterator>(slice.cend());
  CheckType<polly::Span<int>::reverse_iterator>(slice.rend());
  EXPECT_TRUE(
      (std::is_convertible<decltype(slice.rend()),
                           polly::Span<int>::const_reverse_iterator>::value));
  CheckType<polly::Span<int>::const_reverse_iterator>(slice.crend());
  testing::StaticAssertTypeEq<int, polly::Span<int>::value_type>();
  testing::StaticAssertTypeEq<int, polly::Span<const int>::value_type>();
  testing::StaticAssertTypeEq<int, polly::Span<int>::element_type>();
  testing::StaticAssertTypeEq<const int, polly::Span<const int>::element_type>();
  testing::StaticAssertTypeEq<int*, polly::Span<int>::pointer>();
  testing::StaticAssertTypeEq<const int*, polly::Span<const int>::pointer>();
  testing::StaticAssertTypeEq<int&, polly::Span<int>::reference>();
  testing::StaticAssertTypeEq<const int&, polly::Span<const int>::reference>();
  testing::StaticAssertTypeEq<const int&, polly::Span<int>::const_reference>();
  testing::StaticAssertTypeEq<const int&,
                              polly::Span<const int>::const_reference>();
  EXPECT_EQ(static_cast<polly::Span<int>::size_type>(-1), polly::Span<int>::npos);
}

TEST(IntSpan, IteratorsAndReferences) {
  auto accept_pointer = [](int*) {};
  auto accept_reference = [](int&) {};
  auto accept_iterator = [](polly::Span<int>::iterator) {};
  auto accept_const_iterator = [](polly::Span<int>::const_iterator) {};
  auto accept_reverse_iterator = [](polly::Span<int>::reverse_iterator) {};
  auto accept_const_reverse_iterator =
      [](polly::Span<int>::const_reverse_iterator) {};

  int a[1];
  polly::Span<int> s = a;

  accept_pointer(s.data());
  accept_iterator(s.begin());
  accept_const_iterator(s.begin());
  accept_const_iterator(s.cbegin());
  accept_iterator(s.end());
  accept_const_iterator(s.end());
  accept_const_iterator(s.cend());
  accept_reverse_iterator(s.rbegin());
  accept_const_reverse_iterator(s.rbegin());
  accept_const_reverse_iterator(s.crbegin());
  accept_reverse_iterator(s.rend());
  accept_const_reverse_iterator(s.rend());
  accept_const_reverse_iterator(s.crend());

  accept_reference(s[0]);
  accept_reference(s.at(0));
  accept_reference(s.front());
  accept_reference(s.back());
}

TEST(IntSpan, IteratorsAndReferences_Const) {
  auto accept_pointer = [](int*) {};
  auto accept_reference = [](int&) {};
  auto accept_iterator = [](polly::Span<int>::iterator) {};
  auto accept_const_iterator = [](polly::Span<int>::const_iterator) {};
  auto accept_reverse_iterator = [](polly::Span<int>::reverse_iterator) {};
  auto accept_const_reverse_iterator =
      [](polly::Span<int>::const_reverse_iterator) {};

  int a[1];
  const polly::Span<int> s = a;

  accept_pointer(s.data());
  accept_iterator(s.begin());
  accept_const_iterator(s.begin());
  accept_const_iterator(s.cbegin());
  accept_iterator(s.end());
  accept_const_iterator(s.end());
  accept_const_iterator(s.cend());
  accept_reverse_iterator(s.rbegin());
  accept_const_reverse_iterator(s.rbegin());
  accept_const_reverse_iterator(s.crbegin());
  accept_reverse_iterator(s.rend());
  accept_const_reverse_iterator(s.rend());
  accept_const_reverse_iterator(s.crend());

  accept_reference(s[0]);
  accept_reference(s.at(0));
  accept_reference(s.front());
  accept_reference(s.back());
}

TEST(IntSpan, NoexceptTest) {
  int a[] = {1, 2, 3};
  std::vector<int> v;
  EXPECT_TRUE(noexcept(polly::Span<const int>()));
  EXPECT_TRUE(noexcept(polly::Span<const int>(a, 2)));
  EXPECT_TRUE(noexcept(polly::Span<const int>(a)));
  EXPECT_TRUE(noexcept(polly::Span<const int>(v)));
  EXPECT_TRUE(noexcept(polly::Span<int>(v)));
  EXPECT_TRUE(noexcept(polly::Span<const int>({1, 2, 3})));
  EXPECT_TRUE(noexcept(polly::MakeSpan(v)));
  EXPECT_TRUE(noexcept(polly::MakeSpan(a)));
  EXPECT_TRUE(noexcept(polly::MakeSpan(a, 2)));
  EXPECT_TRUE(noexcept(polly::MakeSpan(a, a + 1)));
  EXPECT_TRUE(noexcept(polly::MakeConstSpan(v)));
  EXPECT_TRUE(noexcept(polly::MakeConstSpan(a)));
  EXPECT_TRUE(noexcept(polly::MakeConstSpan(a, 2)));
  EXPECT_TRUE(noexcept(polly::MakeConstSpan(a, a + 1)));

  polly::Span<int> s(v);
  EXPECT_TRUE(noexcept(s.data()));
  EXPECT_TRUE(noexcept(s.size()));
  EXPECT_TRUE(noexcept(s.length()));
  EXPECT_TRUE(noexcept(s.empty()));
  EXPECT_TRUE(noexcept(s[0]));
  EXPECT_TRUE(noexcept(s.front()));
  EXPECT_TRUE(noexcept(s.back()));
  EXPECT_TRUE(noexcept(s.begin()));
  EXPECT_TRUE(noexcept(s.cbegin()));
  EXPECT_TRUE(noexcept(s.end()));
  EXPECT_TRUE(noexcept(s.cend()));
  EXPECT_TRUE(noexcept(s.rbegin()));
  EXPECT_TRUE(noexcept(s.crbegin()));
  EXPECT_TRUE(noexcept(s.rend()));
  EXPECT_TRUE(noexcept(s.crend()));
  EXPECT_TRUE(noexcept(s.remove_prefix(0)));
  EXPECT_TRUE(noexcept(s.remove_suffix(0)));
}

// ConstexprTester exercises expressions in a constexpr context. Simply placing
// the expression in a constexpr function is not enough, as some compilers will
// simply compile the constexpr function as runtime code. Using template
// parameters forces compile-time execution.
template <int i>
struct ConstexprTester {};

#define POLLY_TEST_CONSTEXPR(expr)                       \
  do {                                                  \
    POLLY_ATTR_MAYBE_UNUSED ConstexprTester<(expr, 1)> t; \
  } while (0)

struct ContainerWithConstexprMethods {
  constexpr int size() const { return 1; }
  constexpr const int* data() const { return &i; }
  const int i;
};

TEST(ConstIntSpan, ConstexprTest) {
  static constexpr int a[] = {1, 2, 3};
  static constexpr int sized_arr[2] = {1, 2};
  static constexpr ContainerWithConstexprMethods c{1};
  POLLY_TEST_CONSTEXPR(polly::Span<const int>());
  POLLY_TEST_CONSTEXPR(polly::Span<const int>(a, 2));
  POLLY_TEST_CONSTEXPR(polly::Span<const int>(sized_arr));
  POLLY_TEST_CONSTEXPR(polly::Span<const int>(c));
  POLLY_TEST_CONSTEXPR(polly::MakeSpan(&a[0], 1));
  POLLY_TEST_CONSTEXPR(polly::MakeSpan(c));
  POLLY_TEST_CONSTEXPR(polly::MakeSpan(a));
  POLLY_TEST_CONSTEXPR(polly::MakeConstSpan(&a[0], 1));
  POLLY_TEST_CONSTEXPR(polly::MakeConstSpan(c));
  POLLY_TEST_CONSTEXPR(polly::MakeConstSpan(a));

  constexpr polly::Span<const int> span = c;
  POLLY_TEST_CONSTEXPR(span.data());
  POLLY_TEST_CONSTEXPR(span.size());
  POLLY_TEST_CONSTEXPR(span.length());
  POLLY_TEST_CONSTEXPR(span.empty());
  POLLY_TEST_CONSTEXPR(span.begin());
  POLLY_TEST_CONSTEXPR(span.cbegin());
  POLLY_TEST_CONSTEXPR(span.subspan(0, 0));
  POLLY_TEST_CONSTEXPR(span.first(1));
  POLLY_TEST_CONSTEXPR(span.last(1));
  POLLY_TEST_CONSTEXPR(span[0]);
}

struct BigStruct {
  char bytes[10000];
};

TEST(Span, SpanSize) {
  EXPECT_LE(sizeof(polly::Span<int>), 2 * sizeof(void*));
  EXPECT_LE(sizeof(polly::Span<BigStruct>), 2 * sizeof(void*));
}

}  // namespace
