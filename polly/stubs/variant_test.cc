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

#include "polly/stubs/internal/config.h"

#if !defined(POLLY_HAVE_STD_VARIANT)
#include "polly/stubs/variant.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <ostream>
#include <queue>
#include <type_traits>
#include <unordered_set>
#include <utility>
#include <vector>

#include "polly/stubs/type_traits.h"
#include "polly/stubs/string_view.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#ifdef POLLY_HAVE_EXCEPTIONS

#define POLLY_VARIANT_TEST_EXPECT_FAIL(expr, exception_t, text) \
  EXPECT_THROW(expr, exception_t)

#else

#define POLLY_VARIANT_TEST_EXPECT_FAIL(expr, exception_t, text) \
  EXPECT_DEATH_IF_SUPPORTED(expr, text)

#endif  // POLLY_HAVE_EXCEPTIONS

#define POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(...)                 \
  POLLY_VARIANT_TEST_EXPECT_FAIL((void)(__VA_ARGS__), polly::bad_variant_access, \
                                "Bad variant access")

struct Hashable {};

namespace std {
template <>
struct hash<Hashable> {
  size_t operator()(const Hashable&);
};
}  // namespace std

struct NonHashable {};

namespace polly {
namespace {

using ::testing::DoubleEq;
using ::testing::Pointee;
using ::testing::VariantWith;

struct MoveCanThrow {
  MoveCanThrow() : v(0) {}
  MoveCanThrow(int v) : v(v) {}  // NOLINT(runtime/explicit)
  MoveCanThrow(const MoveCanThrow& other) : v(other.v) {}
  MoveCanThrow& operator=(const MoveCanThrow& /*other*/) { return *this; }
  int v;
};

bool operator==(MoveCanThrow lhs, MoveCanThrow rhs) { return lhs.v == rhs.v; }
bool operator!=(MoveCanThrow lhs, MoveCanThrow rhs) { return lhs.v != rhs.v; }
bool operator<(MoveCanThrow lhs, MoveCanThrow rhs) { return lhs.v < rhs.v; }
bool operator<=(MoveCanThrow lhs, MoveCanThrow rhs) { return lhs.v <= rhs.v; }
bool operator>=(MoveCanThrow lhs, MoveCanThrow rhs) { return lhs.v >= rhs.v; }
bool operator>(MoveCanThrow lhs, MoveCanThrow rhs) { return lhs.v > rhs.v; }

// This helper class allows us to determine if it was swapped with std::swap()
// or with its friend swap() function.
struct SpecialSwap {
  explicit SpecialSwap(int i) : i(i) {}
  void swap(SpecialSwap& a, SpecialSwap& b);
  bool operator==(SpecialSwap other) const { return i == other.i; }
  int i;
  bool special_swap = false;
};

void SpecialSwap::swap(SpecialSwap& a, SpecialSwap& b) {
  a.special_swap = b.special_swap = true;
  std::swap(a.i, b.i);
  abort();
}

struct MoveOnlyWithListConstructor {
  MoveOnlyWithListConstructor() = default;
  explicit MoveOnlyWithListConstructor(std::initializer_list<int> /*ilist*/,
                                       int value)
      : value(value) {}
  MoveOnlyWithListConstructor(MoveOnlyWithListConstructor&&) = default;
  MoveOnlyWithListConstructor& operator=(MoveOnlyWithListConstructor&&) =
      default;

  int value = 0;
};

#ifdef POLLY_HAVE_EXCEPTIONS

struct ConversionException {};

template <class T>
struct ExceptionOnConversion {
  operator T() const {  // NOLINT(runtime/explicit)
    throw ConversionException();
  }
};

// Forces a variant into the valueless by exception state.
template <class H, class... T>
void ToValuelessByException(polly::variant<H, T...>& v) {  // NOLINT
  try {
    v.template emplace<0>(ExceptionOnConversion<H>());
  } catch (ConversionException& /*e*/) {
    // This space intentionally left blank.
  }
}

#endif  // POLLY_HAVE_EXCEPTIONS

// An indexed sequence of distinct structures holding a single
// value of type T
template<typename T, size_t N>
struct ValueHolder {
  explicit ValueHolder(const T& x) : value(x) {}
  typedef T value_type;
  value_type value;
  static const size_t kIndex = N;
};
template<typename T, size_t N>
const size_t ValueHolder<T, N>::kIndex;

// The following three functions make ValueHolder compatible with
// EXPECT_EQ and EXPECT_NE
template<typename T, size_t N>
inline bool operator==(const ValueHolder<T, N>& left,
                       const ValueHolder<T, N>& right) {
  return left.value == right.value;
}

template<typename T, size_t N>
inline bool operator!=(const ValueHolder<T, N>& left,
                       const ValueHolder<T, N>& right) {
  return left.value != right.value;
}

template<typename T, size_t N>
inline std::ostream& operator<<(
    std::ostream& stream, const ValueHolder<T, N>& object) {
  return stream << object.value;
}

// Makes a variant holding twelve uniquely typed T wrappers.
template<typename T>
struct VariantFactory {
  typedef variant<ValueHolder<T, 1>, ValueHolder<T, 2>, ValueHolder<T, 3>,
                  ValueHolder<T, 4>>
      Type;
};

// A typelist in 1:1 with VariantFactory, to use type driven unit tests.
typedef ::testing::Types<ValueHolder<size_t, 1>, ValueHolder<size_t, 2>,
                         ValueHolder<size_t, 3>,
                         ValueHolder<size_t, 4>> VariantTypes;

// Increments the provided counter pointer in the destructor
struct IncrementInDtor {
  explicit IncrementInDtor(int* counter) : counter(counter) {}
  ~IncrementInDtor() { *counter += 1; }
  int* counter;
};

struct IncrementInDtorCopyCanThrow {
  explicit IncrementInDtorCopyCanThrow(int* counter) : counter(counter) {}
  IncrementInDtorCopyCanThrow(IncrementInDtorCopyCanThrow&& other) noexcept =
      default;
  IncrementInDtorCopyCanThrow(const IncrementInDtorCopyCanThrow& other)
      : counter(other.counter) {}
  IncrementInDtorCopyCanThrow& operator=(
      IncrementInDtorCopyCanThrow&&) noexcept = default;
  IncrementInDtorCopyCanThrow& operator=(
      IncrementInDtorCopyCanThrow const& other) {
    counter = other.counter;
    return *this;
  }
  ~IncrementInDtorCopyCanThrow() { *counter += 1; }
  int* counter;
};

// This is defined so operator== for ValueHolder<IncrementInDtor> will
// return true if two IncrementInDtor objects increment the same
// counter
inline bool operator==(const IncrementInDtor& left,
                       const IncrementInDtor& right) {
  return left.counter == right.counter;
}

// This is defined so EXPECT_EQ can work with IncrementInDtor
inline std::ostream& operator<<(
    std::ostream& stream, const IncrementInDtor& object) {
  return stream << object.counter;
}

// A class that can be copied, but not assigned.
class CopyNoAssign {
 public:
  explicit CopyNoAssign(int value) : foo(value) {}
  CopyNoAssign(const CopyNoAssign& other) : foo(other.foo) {}
  int foo;
 private:
  const CopyNoAssign& operator=(const CopyNoAssign&);
};

// A class that can neither be copied nor assigned. We provide
// overloads for the constructor with up to four parameters so we can
// test the overloads of variant::emplace.
class NonCopyable {
 public:
  NonCopyable()
      : value(0) {}
  explicit NonCopyable(int value1)
      : value(value1) {}

  NonCopyable(int value1, int value2)
      : value(value1 + value2) {}

  NonCopyable(int value1, int value2, int value3)
      : value(value1 + value2 + value3) {}

  NonCopyable(int value1, int value2, int value3, int value4)
      : value(value1 + value2 + value3 + value4) {}
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
  int value;
};

// A typed test and typed test case over the VariantTypes typelist,
// from which we derive a number of tests that will execute for one of
// each type.
template <typename T>
class VariantTypesTest : public ::testing::Test {};
TYPED_TEST_SUITE(VariantTypesTest, VariantTypes);

////////////////////
// [variant.ctor] //
////////////////////

struct NonNoexceptDefaultConstructible {
  NonNoexceptDefaultConstructible() {}
  int value = 5;
};

struct NonDefaultConstructible {
  NonDefaultConstructible() = delete;
};

TEST(VariantTest, TestDefaultConstructor) {
  {
    using X = variant<int>;
    constexpr variant<int> x{};
    ASSERT_FALSE(x.valueless_by_exception());
    ASSERT_EQ(0, x.index());
    EXPECT_EQ(0, polly::get<0>(x));
    EXPECT_TRUE(std::is_nothrow_default_constructible<X>::value);
  }

  {
    using X = variant<NonNoexceptDefaultConstructible>;
    X x{};
    ASSERT_FALSE(x.valueless_by_exception());
    ASSERT_EQ(0, x.index());
    EXPECT_EQ(5, polly::get<0>(x).value);
    EXPECT_FALSE(std::is_nothrow_default_constructible<X>::value);
  }

  {
    using X = variant<int, NonNoexceptDefaultConstructible>;
    X x{};
    ASSERT_FALSE(x.valueless_by_exception());
    ASSERT_EQ(0, x.index());
    EXPECT_EQ(0, polly::get<0>(x));
    EXPECT_TRUE(std::is_nothrow_default_constructible<X>::value);
  }

  {
    using X = variant<NonNoexceptDefaultConstructible, int>;
    X x{};
    ASSERT_FALSE(x.valueless_by_exception());
    ASSERT_EQ(0, x.index());
    EXPECT_EQ(5, polly::get<0>(x).value);
    EXPECT_FALSE(std::is_nothrow_default_constructible<X>::value);
  }
  EXPECT_FALSE(
      std::is_default_constructible<variant<NonDefaultConstructible>>::value);
  EXPECT_FALSE((std::is_default_constructible<
                variant<NonDefaultConstructible, int>>::value));
  EXPECT_TRUE((std::is_default_constructible<
               variant<int, NonDefaultConstructible>>::value));
}

// Test that for each slot, copy constructing a variant with that type
// produces a sensible object that correctly reports its type, and
// that copies the provided value.
TYPED_TEST(VariantTypesTest, TestCopyCtor) {
  typedef typename VariantFactory<typename TypeParam::value_type>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  using value_type2 = polly::variant_alternative_t<1, Variant>;
  using value_type3 = polly::variant_alternative_t<2, Variant>;
  using value_type4 = polly::variant_alternative_t<3, Variant>;
  const TypeParam value(TypeParam::kIndex);
  Variant original(value);
  Variant copied(original);
  EXPECT_TRUE(polly::holds_alternative<value_type1>(copied) ||
              TypeParam::kIndex != 1);
  EXPECT_TRUE(polly::holds_alternative<value_type2>(copied) ||
              TypeParam::kIndex != 2);
  EXPECT_TRUE(polly::holds_alternative<value_type3>(copied) ||
              TypeParam::kIndex != 3);
  EXPECT_TRUE(polly::holds_alternative<value_type4>(copied) ||
              TypeParam::kIndex != 4);
  EXPECT_TRUE((polly::get_if<value_type1>(&original) ==
               polly::get_if<value_type1>(&copied)) ||
              TypeParam::kIndex == 1);
  EXPECT_TRUE((polly::get_if<value_type2>(&original) ==
               polly::get_if<value_type2>(&copied)) ||
              TypeParam::kIndex == 2);
  EXPECT_TRUE((polly::get_if<value_type3>(&original) ==
               polly::get_if<value_type3>(&copied)) ||
              TypeParam::kIndex == 3);
  EXPECT_TRUE((polly::get_if<value_type4>(&original) ==
               polly::get_if<value_type4>(&copied)) ||
              TypeParam::kIndex == 4);
  EXPECT_TRUE((polly::get_if<value_type1>(&original) ==
               polly::get_if<value_type1>(&copied)) ||
              TypeParam::kIndex == 1);
  EXPECT_TRUE((polly::get_if<value_type2>(&original) ==
               polly::get_if<value_type2>(&copied)) ||
              TypeParam::kIndex == 2);
  EXPECT_TRUE((polly::get_if<value_type3>(&original) ==
               polly::get_if<value_type3>(&copied)) ||
              TypeParam::kIndex == 3);
  EXPECT_TRUE((polly::get_if<value_type4>(&original) ==
               polly::get_if<value_type4>(&copied)) ||
              TypeParam::kIndex == 4);
  const TypeParam* ovalptr = polly::get_if<TypeParam>(&original);
  const TypeParam* cvalptr = polly::get_if<TypeParam>(&copied);
  ASSERT_TRUE(ovalptr != nullptr);
  ASSERT_TRUE(cvalptr != nullptr);
  EXPECT_EQ(*ovalptr, *cvalptr);
  TypeParam* mutable_ovalptr = polly::get_if<TypeParam>(&original);
  TypeParam* mutable_cvalptr = polly::get_if<TypeParam>(&copied);
  ASSERT_TRUE(mutable_ovalptr != nullptr);
  ASSERT_TRUE(mutable_cvalptr != nullptr);
  EXPECT_EQ(*mutable_ovalptr, *mutable_cvalptr);
}

template <class>
struct MoveOnly {
  MoveOnly() = default;
  explicit MoveOnly(int value) : value(value) {}
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
  int value = 5;
};

TEST(VariantTest, TestMoveConstruct) {
  using V = variant<MoveOnly<class A>, MoveOnly<class B>, MoveOnly<class C>>;

  V v(in_place_index<1>, 10);
  V v2 = std::move(v);
  EXPECT_EQ(10, polly::get<1>(v2).value);
}

// Used internally to emulate missing triviality traits for tests.
template <class T>
union SingleUnion {
  T member;
};

// NOTE: These don't work with types that can't be union members.
//       They are just for testing.
template <class T>
struct is_trivially_move_constructible
    : std::is_move_constructible<SingleUnion<T>>::type {};

template <class T>
struct is_trivially_move_assignable
    : std::is_move_assignable<SingleUnion<T>>::type {};

TEST(VariantTest, NothrowMoveConstructible) {
  // Verify that variant is nothrow move constructible iff its template
  // arguments are.
  using U = std::unique_ptr<int>;
  struct E {
    E(E&&) {}
  };
  static_assert(std::is_nothrow_move_constructible<variant<U>>::value, "");
  static_assert(std::is_nothrow_move_constructible<variant<U, int>>::value, "");
  static_assert(!std::is_nothrow_move_constructible<variant<U, E>>::value, "");
}

// Test that for each slot, constructing a variant with that type
// produces a sensible object that correctly reports its type, and
// that copies the provided value.
TYPED_TEST(VariantTypesTest, TestValueCtor) {
  typedef typename VariantFactory<typename TypeParam::value_type>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  using value_type2 = polly::variant_alternative_t<1, Variant>;
  using value_type3 = polly::variant_alternative_t<2, Variant>;
  using value_type4 = polly::variant_alternative_t<3, Variant>;
  const TypeParam value(TypeParam::kIndex);
  Variant v(value);
  EXPECT_TRUE(polly::holds_alternative<value_type1>(v) ||
              TypeParam::kIndex != 1);
  EXPECT_TRUE(polly::holds_alternative<value_type2>(v) ||
              TypeParam::kIndex != 2);
  EXPECT_TRUE(polly::holds_alternative<value_type3>(v) ||
              TypeParam::kIndex != 3);
  EXPECT_TRUE(polly::holds_alternative<value_type4>(v) ||
              TypeParam::kIndex != 4);
  EXPECT_TRUE(nullptr != polly::get_if<value_type1>(&v) ||
              TypeParam::kIndex != 1);
  EXPECT_TRUE(nullptr != polly::get_if<value_type2>(&v) ||
              TypeParam::kIndex != 2);
  EXPECT_TRUE(nullptr != polly::get_if<value_type3>(&v) ||
              TypeParam::kIndex != 3);
  EXPECT_TRUE(nullptr != polly::get_if<value_type4>(&v) ||
              TypeParam::kIndex != 4);
  EXPECT_TRUE(nullptr != polly::get_if<value_type1>(&v) ||
              TypeParam::kIndex != 1);
  EXPECT_TRUE(nullptr != polly::get_if<value_type2>(&v) ||
              TypeParam::kIndex != 2);
  EXPECT_TRUE(nullptr != polly::get_if<value_type3>(&v) ||
              TypeParam::kIndex != 3);
  EXPECT_TRUE(nullptr != polly::get_if<value_type4>(&v) ||
              TypeParam::kIndex != 4);
  const TypeParam* valptr = polly::get_if<TypeParam>(&v);
  ASSERT_TRUE(nullptr != valptr);
  EXPECT_EQ(value.value, valptr->value);
  const TypeParam* mutable_valptr = polly::get_if<TypeParam>(&v);
  ASSERT_TRUE(nullptr != mutable_valptr);
  EXPECT_EQ(value.value, mutable_valptr->value);
}

TEST(VariantTest, AmbiguousValueConstructor) {
  EXPECT_FALSE((std::is_convertible<int, polly::variant<int, int>>::value));
  EXPECT_FALSE((std::is_constructible<polly::variant<int, int>, int>::value));
}

TEST(VariantTest, InPlaceType) {
  using Var = variant<int, std::string, NonCopyable, std::vector<int>>;

  Var v1(in_place_type_t<int>(), 7);
  ASSERT_TRUE(polly::holds_alternative<int>(v1));
  EXPECT_EQ(7, polly::get<int>(v1));

  Var v2(in_place_type_t<std::string>(), "ABC");
  ASSERT_TRUE(polly::holds_alternative<std::string>(v2));
  EXPECT_EQ("ABC", polly::get<std::string>(v2));

  Var v3(in_place_type_t<std::string>(), "ABC", 2);
  ASSERT_TRUE(polly::holds_alternative<std::string>(v3));
  EXPECT_EQ("AB", polly::get<std::string>(v3));

  Var v4(in_place_type_t<NonCopyable>{});
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v4));

  Var v5(in_place_type_t<std::vector<int>>(), {1, 2, 3});
  ASSERT_TRUE(polly::holds_alternative<std::vector<int>>(v5));
  EXPECT_THAT(polly::get<std::vector<int>>(v5), ::testing::ElementsAre(1, 2, 3));
}

TEST(VariantTest, InPlaceTypeVariableTemplate) {
  using Var = variant<int, std::string, NonCopyable, std::vector<int>>;

  Var v1(in_place_type<int>, 7);
  ASSERT_TRUE(polly::holds_alternative<int>(v1));
  EXPECT_EQ(7, polly::get<int>(v1));

  Var v2(in_place_type<std::string>, "ABC");
  ASSERT_TRUE(polly::holds_alternative<std::string>(v2));
  EXPECT_EQ("ABC", polly::get<std::string>(v2));

  Var v3(in_place_type<std::string>, "ABC", 2);
  ASSERT_TRUE(polly::holds_alternative<std::string>(v3));
  EXPECT_EQ("AB", polly::get<std::string>(v3));

  Var v4(in_place_type<NonCopyable>);
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v4));

  Var v5(in_place_type<std::vector<int>>, {1, 2, 3});
  ASSERT_TRUE(polly::holds_alternative<std::vector<int>>(v5));
  EXPECT_THAT(polly::get<std::vector<int>>(v5), ::testing::ElementsAre(1, 2, 3));
}

TEST(VariantTest, InPlaceTypeInitializerList) {
  using Var =
      variant<int, std::string, NonCopyable, MoveOnlyWithListConstructor>;

  Var v1(in_place_type_t<MoveOnlyWithListConstructor>(), {1, 2, 3, 4, 5}, 6);
  ASSERT_TRUE(polly::holds_alternative<MoveOnlyWithListConstructor>(v1));
  EXPECT_EQ(6, polly::get<MoveOnlyWithListConstructor>(v1).value);
}

TEST(VariantTest, InPlaceTypeInitializerListVariabletemplate) {
  using Var =
      variant<int, std::string, NonCopyable, MoveOnlyWithListConstructor>;

  Var v1(in_place_type<MoveOnlyWithListConstructor>, {1, 2, 3, 4, 5}, 6);
  ASSERT_TRUE(polly::holds_alternative<MoveOnlyWithListConstructor>(v1));
  EXPECT_EQ(6, polly::get<MoveOnlyWithListConstructor>(v1).value);
}

TEST(VariantTest, InPlaceIndex) {
  using Var = variant<int, std::string, NonCopyable, std::vector<int>>;

  Var v1(in_place_index_t<0>(), 7);
  ASSERT_TRUE(polly::holds_alternative<int>(v1));
  EXPECT_EQ(7, polly::get<int>(v1));

  Var v2(in_place_index_t<1>(), "ABC");
  ASSERT_TRUE(polly::holds_alternative<std::string>(v2));
  EXPECT_EQ("ABC", polly::get<std::string>(v2));

  Var v3(in_place_index_t<1>(), "ABC", 2);
  ASSERT_TRUE(polly::holds_alternative<std::string>(v3));
  EXPECT_EQ("AB", polly::get<std::string>(v3));

  Var v4(in_place_index_t<2>{});
  EXPECT_TRUE(polly::holds_alternative<NonCopyable>(v4));

  // Verify that a variant with only non-copyables can still be constructed.
  EXPECT_TRUE(polly::holds_alternative<NonCopyable>(
      variant<NonCopyable>(in_place_index_t<0>{})));

  Var v5(in_place_index_t<3>(), {1, 2, 3});
  ASSERT_TRUE(polly::holds_alternative<std::vector<int>>(v5));
  EXPECT_THAT(polly::get<std::vector<int>>(v5), ::testing::ElementsAre(1, 2, 3));
}

TEST(VariantTest, InPlaceIndexVariableTemplate) {
  using Var = variant<int, std::string, NonCopyable, std::vector<int>>;

  Var v1(in_place_index<0>, 7);
  ASSERT_TRUE(polly::holds_alternative<int>(v1));
  EXPECT_EQ(7, polly::get<int>(v1));

  Var v2(in_place_index<1>, "ABC");
  ASSERT_TRUE(polly::holds_alternative<std::string>(v2));
  EXPECT_EQ("ABC", polly::get<std::string>(v2));

  Var v3(in_place_index<1>, "ABC", 2);
  ASSERT_TRUE(polly::holds_alternative<std::string>(v3));
  EXPECT_EQ("AB", polly::get<std::string>(v3));

  Var v4(in_place_index<2>);
  EXPECT_TRUE(polly::holds_alternative<NonCopyable>(v4));

  // Verify that a variant with only non-copyables can still be constructed.
  EXPECT_TRUE(polly::holds_alternative<NonCopyable>(
      variant<NonCopyable>(in_place_index<0>)));

  Var v5(in_place_index<3>, {1, 2, 3});
  ASSERT_TRUE(polly::holds_alternative<std::vector<int>>(v5));
  EXPECT_THAT(polly::get<std::vector<int>>(v5), ::testing::ElementsAre(1, 2, 3));
}

TEST(VariantTest, InPlaceIndexInitializerList) {
  using Var =
      variant<int, std::string, NonCopyable, MoveOnlyWithListConstructor>;

  Var v1(in_place_index_t<3>(), {1, 2, 3, 4, 5}, 6);
  ASSERT_TRUE(polly::holds_alternative<MoveOnlyWithListConstructor>(v1));
  EXPECT_EQ(6, polly::get<MoveOnlyWithListConstructor>(v1).value);
}

TEST(VariantTest, InPlaceIndexInitializerListVariableTemplate) {
  using Var =
      variant<int, std::string, NonCopyable, MoveOnlyWithListConstructor>;

  Var v1(in_place_index<3>, {1, 2, 3, 4, 5}, 6);
  ASSERT_TRUE(polly::holds_alternative<MoveOnlyWithListConstructor>(v1));
  EXPECT_EQ(6, polly::get<MoveOnlyWithListConstructor>(v1).value);
}

////////////////////
// [variant.dtor] //
////////////////////

// Make sure that the destructor destroys the contained value
TEST(VariantTest, TestDtor) {
  typedef VariantFactory<IncrementInDtor>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  using value_type2 = polly::variant_alternative_t<1, Variant>;
  using value_type3 = polly::variant_alternative_t<2, Variant>;
  using value_type4 = polly::variant_alternative_t<3, Variant>;
  int counter = 0;
  IncrementInDtor counter_adjuster(&counter);
  EXPECT_EQ(0, counter);

  value_type1 value1(counter_adjuster);
  { Variant object(value1); }
  EXPECT_EQ(1, counter);

  value_type2 value2(counter_adjuster);
  { Variant object(value2); }
  EXPECT_EQ(2, counter);

  value_type3 value3(counter_adjuster);
  { Variant object(value3); }
  EXPECT_EQ(3, counter);

  value_type4 value4(counter_adjuster);
  { Variant object(value4); }
  EXPECT_EQ(4, counter);
}

#ifdef POLLY_HAVE_EXCEPTIONS
// Test destruction when in the valueless_by_exception state.
TEST(VariantTest, TestDtorValuelessByException)
{
  int counter = 0;
  IncrementInDtor counter_adjuster(&counter);

  {
    using Variant = VariantFactory<IncrementInDtor>::Type;

    Variant v(in_place_index<0>, counter_adjuster);
    EXPECT_EQ(0, counter);

    ToValuelessByException(v);
    ASSERT_TRUE(v.valueless_by_exception());
    EXPECT_EQ(1, counter);
  }
  EXPECT_EQ(1, counter);
}

#endif  // POLLY_HAVE_EXCEPTIONS

//////////////////////
// [variant.assign] //
//////////////////////

// Test that self-assignment doesn't destroy the current value
TEST(VariantTest, TestSelfAssignment) {
  typedef VariantFactory<IncrementInDtor>::Type Variant;
  int counter = 0;
  IncrementInDtor counter_adjuster(&counter);
  polly::variant_alternative_t<0, Variant> value(counter_adjuster);
  Variant object(value);
  object.operator=(object);
  EXPECT_EQ(0, counter);

  // A string long enough that it's likely to defeat any inline representation
  // optimization.
  const std::string long_str(128, 'a');

  std::string foo = long_str;
  foo = *&foo;
  EXPECT_EQ(long_str, foo);

  variant<int, std::string> so = long_str;
  ASSERT_EQ(1, so.index());
  EXPECT_EQ(long_str, polly::get<1>(so));
  so = *&so;

  ASSERT_EQ(1, so.index());
  EXPECT_EQ(long_str, polly::get<1>(so));
}

// Test that assigning a variant<..., T, ...> to a variant<..., T, ...> produces
// a variant<..., T, ...> with the correct value.
TYPED_TEST(VariantTypesTest, TestAssignmentCopiesValueSameTypes) {
  typedef typename VariantFactory<typename TypeParam::value_type>::Type Variant;
  const TypeParam value(TypeParam::kIndex);
  const Variant source(value);
  Variant target(TypeParam(value.value + 1));
  ASSERT_TRUE(polly::holds_alternative<TypeParam>(source));
  ASSERT_TRUE(polly::holds_alternative<TypeParam>(target));
  ASSERT_NE(polly::get<TypeParam>(source), polly::get<TypeParam>(target));
  target = source;
  ASSERT_TRUE(polly::holds_alternative<TypeParam>(source));
  ASSERT_TRUE(polly::holds_alternative<TypeParam>(target));
  EXPECT_EQ(polly::get<TypeParam>(source), polly::get<TypeParam>(target));
}

// Test that assisnging a variant<..., T, ...> to a variant<1, ...>
// produces a variant<..., T, ...> with the correct value.
TYPED_TEST(VariantTypesTest, TestAssignmentCopiesValuesVaryingSourceType) {
  typedef typename VariantFactory<typename TypeParam::value_type>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  const TypeParam value(TypeParam::kIndex);
  const Variant source(value);
  ASSERT_TRUE(polly::holds_alternative<TypeParam>(source));
  Variant target(value_type1(1));
  ASSERT_TRUE(polly::holds_alternative<value_type1>(target));
  target = source;
  EXPECT_TRUE(polly::holds_alternative<TypeParam>(source));
  EXPECT_TRUE(polly::holds_alternative<TypeParam>(target));
  EXPECT_EQ(polly::get<TypeParam>(source), polly::get<TypeParam>(target));
}

// Test that assigning a variant<1, ...> to a variant<..., T, ...>
// produces a variant<1, ...> with the correct value.
TYPED_TEST(VariantTypesTest, TestAssignmentCopiesValuesVaryingTargetType) {
  typedef typename VariantFactory<typename TypeParam::value_type>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  const Variant source(value_type1(1));
  ASSERT_TRUE(polly::holds_alternative<value_type1>(source));
  const TypeParam value(TypeParam::kIndex);
  Variant target(value);
  ASSERT_TRUE(polly::holds_alternative<TypeParam>(target));
  target = source;
  EXPECT_TRUE(polly::holds_alternative<value_type1>(target));
  EXPECT_TRUE(polly::holds_alternative<value_type1>(source));
  EXPECT_EQ(polly::get<value_type1>(source), polly::get<value_type1>(target));
}

// Test that operator=<T> works, that assigning a new value destroys
// the old and that assigning the new value again does not redestroy
// the old
TEST(VariantTest, TestAssign) {
  typedef VariantFactory<IncrementInDtor>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  using value_type2 = polly::variant_alternative_t<1, Variant>;
  using value_type3 = polly::variant_alternative_t<2, Variant>;
  using value_type4 = polly::variant_alternative_t<3, Variant>;

  const int kSize = 4;
  int counter[kSize];
  std::unique_ptr<IncrementInDtor> counter_adjustor[kSize];
  for (int i = 0; i != kSize; i++) {
    counter[i] = 0;
    counter_adjustor[i] = std::unique_ptr<IncrementInDtor>(new IncrementInDtor(&counter[i]));
  }

  value_type1 v1(*counter_adjustor[0]);
  value_type2 v2(*counter_adjustor[1]);
  value_type3 v3(*counter_adjustor[2]);
  value_type4 v4(*counter_adjustor[3]);

  // Test that reassignment causes destruction of old value
  {
    Variant object(v1);
    object = v2;
    object = v3;
    object = v4;
    object = v1;
  }

  EXPECT_EQ(2, counter[0]);
  EXPECT_EQ(1, counter[1]);
  EXPECT_EQ(1, counter[2]);
  EXPECT_EQ(1, counter[3]);

  std::fill(std::begin(counter), std::end(counter), 0);

  // Test that self-assignment does not cause destruction of old value
  {
    Variant object(v1);
    object.operator=(object);
    EXPECT_EQ(0, counter[0]);
  }
  {
    Variant object(v2);
    object.operator=(object);
    EXPECT_EQ(0, counter[1]);
  }
  {
    Variant object(v3);
    object.operator=(object);
    EXPECT_EQ(0, counter[2]);
  }
  {
    Variant object(v4);
    object.operator=(object);
    EXPECT_EQ(0, counter[3]);
  }

  EXPECT_EQ(1, counter[0]);
  EXPECT_EQ(1, counter[1]);
  EXPECT_EQ(1, counter[2]);
  EXPECT_EQ(1, counter[3]);
}

// This tests that we perform a backup if the copy-assign can throw but the move
// cannot throw.
TEST(VariantTest, TestBackupAssign) {
  typedef VariantFactory<IncrementInDtorCopyCanThrow>::Type Variant;
  using value_type1 = polly::variant_alternative_t<0, Variant>;
  using value_type2 = polly::variant_alternative_t<1, Variant>;
  using value_type3 = polly::variant_alternative_t<2, Variant>;
  using value_type4 = polly::variant_alternative_t<3, Variant>;

  const int kSize = 4;
  int counter[kSize];
  std::unique_ptr<IncrementInDtorCopyCanThrow> counter_adjustor[kSize];
  for (int i = 0; i != kSize; i++) {
    counter[i] = 0;
    counter_adjustor[i].reset(new IncrementInDtorCopyCanThrow(&counter[i]));
  }

  value_type1 v1(*counter_adjustor[0]);
  value_type2 v2(*counter_adjustor[1]);
  value_type3 v3(*counter_adjustor[2]);
  value_type4 v4(*counter_adjustor[3]);

  // Test that reassignment causes destruction of old value
  {
    Variant object(v1);
    object = v2;
    object = v3;
    object = v4;
    object = v1;
  }

  // libstdc++ doesn't pass this test
#if !(defined(POLLY_HAVE_STD_VARIANT) && defined(__GLIBCXX__))
  EXPECT_EQ(3, counter[0]);
  EXPECT_EQ(2, counter[1]);
  EXPECT_EQ(2, counter[2]);
  EXPECT_EQ(2, counter[3]);
#endif

  std::fill(std::begin(counter), std::end(counter), 0);

  // Test that self-assignment does not cause destruction of old value
  {
    Variant object(v1);
    object.operator=(object);
    EXPECT_EQ(0, counter[0]);
  }
  {
    Variant object(v2);
    object.operator=(object);
    EXPECT_EQ(0, counter[1]);
  }
  {
    Variant object(v3);
    object.operator=(object);
    EXPECT_EQ(0, counter[2]);
  }
  {
    Variant object(v4);
    object.operator=(object);
    EXPECT_EQ(0, counter[3]);
  }

  EXPECT_EQ(1, counter[0]);
  EXPECT_EQ(1, counter[1]);
  EXPECT_EQ(1, counter[2]);
  EXPECT_EQ(1, counter[3]);
}

///////////////////
// [variant.mod] //
///////////////////

TEST(VariantTest, TestEmplaceBasic) {
  using Variant = variant<int, char>;

  Variant v(polly::in_place_index<0>, 0);

  {
    char& emplace_result = v.emplace<char>();
    ASSERT_TRUE(polly::holds_alternative<char>(v));
    EXPECT_EQ(polly::get<char>(v), 0);
    EXPECT_EQ(&emplace_result, &polly::get<char>(v));
  }

  // Make sure that another emplace does zero-initialization
  polly::get<char>(v) = 'a';
  v.emplace<char>('b');
  ASSERT_TRUE(polly::holds_alternative<char>(v));
  EXPECT_EQ(polly::get<char>(v), 'b');

  {
    int& emplace_result = v.emplace<int>();
    EXPECT_TRUE(polly::holds_alternative<int>(v));
    EXPECT_EQ(polly::get<int>(v), 0);
    EXPECT_EQ(&emplace_result, &polly::get<int>(v));
  }
}

TEST(VariantTest, TestEmplaceInitializerList) {
  using Var =
      variant<int, std::string, NonCopyable, MoveOnlyWithListConstructor>;

  Var v1(polly::in_place_index<0>, 555);
  MoveOnlyWithListConstructor& emplace_result =
      v1.emplace<MoveOnlyWithListConstructor>({1, 2, 3, 4, 5}, 6);
  ASSERT_TRUE(polly::holds_alternative<MoveOnlyWithListConstructor>(v1));
  EXPECT_EQ(6, polly::get<MoveOnlyWithListConstructor>(v1).value);
  EXPECT_EQ(&emplace_result, &polly::get<MoveOnlyWithListConstructor>(v1));
}

TEST(VariantTest, TestEmplaceIndex) {
  using Variant = variant<int, char>;

  Variant v(polly::in_place_index<0>, 555);

  {
    char& emplace_result = v.emplace<1>();
    ASSERT_TRUE(polly::holds_alternative<char>(v));
    EXPECT_EQ(polly::get<char>(v), 0);
    EXPECT_EQ(&emplace_result, &polly::get<char>(v));
  }

  // Make sure that another emplace does zero-initialization
  polly::get<char>(v) = 'a';
  v.emplace<1>('b');
  ASSERT_TRUE(polly::holds_alternative<char>(v));
  EXPECT_EQ(polly::get<char>(v), 'b');

  {
    int& emplace_result = v.emplace<0>();
    EXPECT_TRUE(polly::holds_alternative<int>(v));
    EXPECT_EQ(polly::get<int>(v), 0);
    EXPECT_EQ(&emplace_result, &polly::get<int>(v));
  }
}

TEST(VariantTest, TestEmplaceIndexInitializerList) {
  using Var =
      variant<int, std::string, NonCopyable, MoveOnlyWithListConstructor>;

  Var v1(polly::in_place_index<0>, 555);
  MoveOnlyWithListConstructor& emplace_result =
      v1.emplace<3>({1, 2, 3, 4, 5}, 6);
  ASSERT_TRUE(polly::holds_alternative<MoveOnlyWithListConstructor>(v1));
  EXPECT_EQ(6, polly::get<MoveOnlyWithListConstructor>(v1).value);
  EXPECT_EQ(&emplace_result, &polly::get<MoveOnlyWithListConstructor>(v1));
}

//////////////////////
// [variant.status] //
//////////////////////

TEST(VariantTest, Index) {
  using Var = variant<int, std::string, double>;

  Var v = 1;
  EXPECT_EQ(0, v.index());
  v = "str";
  EXPECT_EQ(1, v.index());
  v = 0.;
  EXPECT_EQ(2, v.index());

  Var v2 = v;
  EXPECT_EQ(2, v2.index());
  v2.emplace<int>(3);
  EXPECT_EQ(0, v2.index());
}

TEST(VariantTest, NotValuelessByException) {
  using Var = variant<int, std::string, double>;

  Var v = 1;
  EXPECT_FALSE(v.valueless_by_exception());
  v = "str";
  EXPECT_FALSE(v.valueless_by_exception());
  v = 0.;
  EXPECT_FALSE(v.valueless_by_exception());

  Var v2 = v;
  EXPECT_FALSE(v.valueless_by_exception());
  v2.emplace<int>(3);
  EXPECT_FALSE(v.valueless_by_exception());
}

#ifdef POLLY_HAVE_EXCEPTIONS

TEST(VariantTest, IndexValuelessByException) {
  using Var = variant<MoveCanThrow, std::string, double>;

  Var v(polly::in_place_index<0>);
  EXPECT_EQ(0, v.index());
  ToValuelessByException(v);
  EXPECT_EQ(polly::variant_npos, v.index());
  v = "str";
  EXPECT_EQ(1, v.index());
}

TEST(VariantTest, ValuelessByException) {
  using Var = variant<MoveCanThrow, std::string, double>;

  Var v(polly::in_place_index<0>);
  EXPECT_FALSE(v.valueless_by_exception());
  ToValuelessByException(v);
  EXPECT_TRUE(v.valueless_by_exception());
  v = "str";
  EXPECT_FALSE(v.valueless_by_exception());
}

#endif  // POLLY_HAVE_EXCEPTIONS

////////////////////
// [variant.swap] //
////////////////////

TEST(VariantTest, MemberSwap) {
  SpecialSwap v1(3);
  SpecialSwap v2(7);

  variant<SpecialSwap> a = v1, b = v2;

  EXPECT_THAT(a, VariantWith<SpecialSwap>(v1));
  EXPECT_THAT(b, VariantWith<SpecialSwap>(v2));

  a.swap(b);
  EXPECT_THAT(a, VariantWith<SpecialSwap>(v2));
  EXPECT_THAT(b, VariantWith<SpecialSwap>(v1));
  // TODO
  //EXPECT_TRUE(polly::get<SpecialSwap>(a).special_swap);

  using V = variant<MoveCanThrow, std::string, int>;
  int i = 33;
  std::string s = "abc";
  {
    // lhs and rhs holds different alternative
    V lhs(i), rhs(s);
    lhs.swap(rhs);
    EXPECT_THAT(lhs, VariantWith<std::string>(s));
    EXPECT_THAT(rhs, VariantWith<int>(i));
  }
#ifdef POLLY_HAVE_EXCEPTIONS
  V valueless(in_place_index<0>);
  ToValuelessByException(valueless);
  {
    // lhs is valueless
    V lhs(valueless), rhs(i);
    lhs.swap(rhs);
    EXPECT_THAT(lhs, VariantWith<int>(i));
    EXPECT_TRUE(rhs.valueless_by_exception());
  }
  {
    // rhs is valueless
    V lhs(s), rhs(valueless);
    lhs.swap(rhs);
    EXPECT_THAT(rhs, VariantWith<std::string>(s));
    EXPECT_TRUE(lhs.valueless_by_exception());
  }
  {
    // both are valueless
    V lhs(valueless), rhs(valueless);
    lhs.swap(rhs);
    EXPECT_TRUE(lhs.valueless_by_exception());
    EXPECT_TRUE(rhs.valueless_by_exception());
  }
#endif  // POLLY_HAVE_EXCEPTIONS
}

//////////////////////
// [variant.helper] //
//////////////////////

TEST(VariantTest, VariantSize) {
  {
    using Size1Variant = polly::variant<int>;
    EXPECT_EQ(1, polly::variant_size<Size1Variant>::value);
    EXPECT_EQ(1, polly::variant_size<const Size1Variant>::value);
    EXPECT_EQ(1, polly::variant_size<volatile Size1Variant>::value);
    EXPECT_EQ(1, polly::variant_size<const volatile Size1Variant>::value);
  }

  {
    using Size3Variant = polly::variant<int, float, int>;
    EXPECT_EQ(3, polly::variant_size<Size3Variant>::value);
    EXPECT_EQ(3, polly::variant_size<const Size3Variant>::value);
    EXPECT_EQ(3, polly::variant_size<volatile Size3Variant>::value);
    EXPECT_EQ(3, polly::variant_size<const volatile Size3Variant>::value);
  }
}

TEST(VariantTest, VariantAlternative) {
  {
    using V = polly::variant<float, int, const char*>;
    EXPECT_TRUE((std::is_same<float, polly::variant_alternative_t<0, V>>::value));
    EXPECT_TRUE((std::is_same<const float, polly::variant_alternative_t<0, const V>>::value));
    EXPECT_TRUE((std::is_same<volatile float, polly::variant_alternative_t<0, volatile V>>::value));
    EXPECT_TRUE((std::is_same<const volatile float, polly::variant_alternative_t<0, const volatile V>>::value));

    EXPECT_TRUE((std::is_same<int, polly::variant_alternative_t<1, V>>::value));
    EXPECT_TRUE((std::is_same<const int, polly::variant_alternative_t<1, const V>>::value));
    EXPECT_TRUE((std::is_same<volatile int, polly::variant_alternative_t<1, volatile V>>::value));
    EXPECT_TRUE((std::is_same<const volatile int, polly::variant_alternative_t<1, const volatile V>>::value));

    EXPECT_TRUE((std::is_same<const char*, polly::variant_alternative_t<2, V>>::value));
    EXPECT_TRUE((std::is_same<const char* const, polly::variant_alternative_t<2, const V>>::value));
    EXPECT_TRUE((std::is_same<const char* volatile, polly::variant_alternative_t<2, volatile V>>::value));
    EXPECT_TRUE((std::is_same<const char* const volatile, polly::variant_alternative_t<2, const volatile V>>::value));
  }

  {
    using V = polly::variant<float, volatile int, const char*>;
    EXPECT_TRUE((std::is_same<float, polly::variant_alternative_t<0, V>>::value));
    EXPECT_TRUE((std::is_same<const float, polly::variant_alternative_t<0, const V>>::value));
    EXPECT_TRUE((std::is_same<volatile float, polly::variant_alternative_t<0, volatile V>>::value));
    EXPECT_TRUE((std::is_same<const volatile float, polly::variant_alternative_t<0, const volatile V>>::value));

    EXPECT_TRUE((std::is_same<volatile int, polly::variant_alternative_t<1, V>>::value));
    EXPECT_TRUE((std::is_same<const volatile int, polly::variant_alternative_t<1, const V>>::value));
    EXPECT_TRUE((std::is_same<volatile int, polly::variant_alternative_t<1, volatile V>>::value));
    EXPECT_TRUE((std::is_same<const volatile int, polly::variant_alternative_t<1, const volatile V>>::value));

    EXPECT_TRUE((std::is_same<const char*, polly::variant_alternative_t<2, V>>::value));
    EXPECT_TRUE((std::is_same<const char* const, polly::variant_alternative_t<2, const V>>::value));
    EXPECT_TRUE((std::is_same<const char* volatile, polly::variant_alternative_t<2, volatile V>>::value));
    EXPECT_TRUE((std::is_same<const char* const volatile, polly::variant_alternative_t<2, const volatile V>>::value));
  }
}

///////////////////
// [variant.get] //
///////////////////

TEST(VariantTest, HoldsAlternative) {
  using Var = variant<int, std::string, double>;

  Var v = 1;
  EXPECT_TRUE(polly::holds_alternative<int>(v));
  EXPECT_FALSE(polly::holds_alternative<std::string>(v));
  EXPECT_FALSE(polly::holds_alternative<double>(v));
  v = "str";
  EXPECT_FALSE(polly::holds_alternative<int>(v));
  EXPECT_TRUE(polly::holds_alternative<std::string>(v));
  EXPECT_FALSE(polly::holds_alternative<double>(v));
  v = 0.;
  EXPECT_FALSE(polly::holds_alternative<int>(v));
  EXPECT_FALSE(polly::holds_alternative<std::string>(v));
  EXPECT_TRUE(polly::holds_alternative<double>(v));

  Var v2 = v;
  EXPECT_FALSE(polly::holds_alternative<int>(v2));
  EXPECT_FALSE(polly::holds_alternative<std::string>(v2));
  EXPECT_TRUE(polly::holds_alternative<double>(v2));
  v2.emplace<int>(3);
  EXPECT_TRUE(polly::holds_alternative<int>(v2));
  EXPECT_FALSE(polly::holds_alternative<std::string>(v2));
  EXPECT_FALSE(polly::holds_alternative<double>(v2));
}

TEST(VariantTest, GetIndex) {
  using Var = variant<int, std::string, double, int>;

  {
    Var v(polly::in_place_index<0>, 0);

    using LValueGetType = decltype(polly::get<0>(v));
    using RValueGetType = decltype(polly::get<0>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, int&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, int&&>::value));
    EXPECT_EQ(polly::get<0>(v), 0);
    EXPECT_EQ(polly::get<0>(std::move(v)), 0);

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<0>(const_v));
    using ConstRValueGetType = decltype(polly::get<0>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const int&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const int&&>::value));
    EXPECT_EQ(polly::get<0>(const_v), 0);
    EXPECT_EQ(polly::get<0>(std::move(const_v)), 0);
  }

  {
    Var v = std::string("Hello");

    using LValueGetType = decltype(polly::get<1>(v));
    using RValueGetType = decltype(polly::get<1>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, std::string&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, std::string&&>::value));
    EXPECT_EQ(polly::get<1>(v), "Hello");
    EXPECT_EQ(polly::get<1>(std::move(v)), "Hello");

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<1>(const_v));
    using ConstRValueGetType = decltype(polly::get<1>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const std::string&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const std::string&&>::value));
    EXPECT_EQ(polly::get<1>(const_v), "Hello");
    EXPECT_EQ(polly::get<1>(std::move(const_v)), "Hello");
  }

  {
    Var v = 2.0;

    using LValueGetType = decltype(polly::get<2>(v));
    using RValueGetType = decltype(polly::get<2>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, double&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, double&&>::value));
    EXPECT_EQ(polly::get<2>(v), 2.);
    EXPECT_EQ(polly::get<2>(std::move(v)), 2.);

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<2>(const_v));
    using ConstRValueGetType = decltype(polly::get<2>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const double&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const double&&>::value));
    EXPECT_EQ(polly::get<2>(const_v), 2.);
    EXPECT_EQ(polly::get<2>(std::move(const_v)), 2.);
  }

  {
    Var v(polly::in_place_index<0>, 0);
    v.emplace<3>(1);

    using LValueGetType = decltype(polly::get<3>(v));
    using RValueGetType = decltype(polly::get<3>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, int&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, int&&>::value));
    EXPECT_EQ(polly::get<3>(v), 1);
    EXPECT_EQ(polly::get<3>(std::move(v)), 1);

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<3>(const_v));
    using ConstRValueGetType = decltype(polly::get<3>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const int&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const int&&>::value));
    EXPECT_EQ(polly::get<3>(const_v), 1);
    EXPECT_EQ(polly::get<3>(std::move(const_v)), 1);  // NOLINT
  }
}

TEST(VariantTest, BadGetIndex) {
#if defined(POLLY_HAVE_EXCEPTIONS)
  using Var = variant<int, std::string, double>;

  {
    Var v = 1;

    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<1>(v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<1>(std::move(v)));

    const Var& const_v = v;
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<1>(const_v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(
        polly::get<1>(std::move(const_v)));  // NOLINT
  }

  {
    Var v = std::string("Hello");

    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<0>(v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<0>(std::move(v)));

    const Var& const_v = v;
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<0>(const_v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(
        polly::get<0>(std::move(const_v)));  // NOLINT
  }
#endif // POLLY_HAVE_EXCEPTIONS
}

TEST(VariantTest, GetType) {
  using Var = variant<int, std::string, double>;

  {
    Var v = 1;

    using LValueGetType = decltype(polly::get<int>(v));
    using RValueGetType = decltype(polly::get<int>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, int&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, int&&>::value));
    EXPECT_EQ(polly::get<int>(v), 1);
    EXPECT_EQ(polly::get<int>(std::move(v)), 1);

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<int>(const_v));
    using ConstRValueGetType = decltype(polly::get<int>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const int&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const int&&>::value));
    EXPECT_EQ(polly::get<int>(const_v), 1);
    EXPECT_EQ(polly::get<int>(std::move(const_v)), 1);
  }

  {
    Var v = std::string("Hello");

    using LValueGetType = decltype(polly::get<1>(v));
    using RValueGetType = decltype(polly::get<1>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, std::string&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, std::string&&>::value));
    EXPECT_EQ(polly::get<std::string>(v), "Hello");
    EXPECT_EQ(polly::get<std::string>(std::move(v)), "Hello");

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<1>(const_v));
    using ConstRValueGetType = decltype(polly::get<1>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const std::string&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const std::string&&>::value));
    EXPECT_EQ(polly::get<std::string>(const_v), "Hello");
    EXPECT_EQ(polly::get<std::string>(std::move(const_v)), "Hello");
  }

  {
    Var v = 2.0;

    using LValueGetType = decltype(polly::get<2>(v));
    using RValueGetType = decltype(polly::get<2>(std::move(v)));

    EXPECT_TRUE((std::is_same<LValueGetType, double&>::value));
    EXPECT_TRUE((std::is_same<RValueGetType, double&&>::value));
    EXPECT_EQ(polly::get<double>(v), 2.);
    EXPECT_EQ(polly::get<double>(std::move(v)), 2.);

    const Var& const_v = v;
    using ConstLValueGetType = decltype(polly::get<2>(const_v));
    using ConstRValueGetType = decltype(polly::get<2>(std::move(const_v)));
    EXPECT_TRUE((std::is_same<ConstLValueGetType, const double&>::value));
    EXPECT_TRUE((std::is_same<ConstRValueGetType, const double&&>::value));
    EXPECT_EQ(polly::get<double>(const_v), 2.);
    EXPECT_EQ(polly::get<double>(std::move(const_v)), 2.);
  }
}

TEST(VariantTest, BadGetType) {
#if defined(POLLY_HAVE_EXCEPTIONS)
  using Var = variant<int, std::string, double>;

  {
    Var v = 1;

    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<std::string>(v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(
        polly::get<std::string>(std::move(v)));

    const Var& const_v = v;
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(
        polly::get<std::string>(const_v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(
        polly::get<std::string>(std::move(const_v)));  // NOLINT
  }

  {
    Var v = std::string("Hello");

    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<int>(v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<int>(std::move(v)));

    const Var& const_v = v;
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(polly::get<int>(const_v));
    POLLY_VARIANT_TEST_EXPECT_BAD_VARIANT_ACCESS(
        polly::get<int>(std::move(const_v)));  // NOLINT
  }
#endif // POLLY_HAVE_EXCEPTIONS
}

TEST(VariantTest, GetIfIndex) {
  using Var = variant<int, std::string, double, int>;

  {
    Var v(polly::in_place_index<0>, 0);
    EXPECT_TRUE(noexcept(polly::get_if<0>(&v)));

    {
      auto* elem = polly::get_if<0>(&v);
      EXPECT_TRUE((std::is_same<decltype(elem), int*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, 0);
      {
        auto* bad_elem = polly::get_if<1>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), std::string*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<2>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), double*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<3>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
    }

    const Var& const_v = v;
    EXPECT_TRUE(noexcept(polly::get_if<0>(&const_v)));

    {
      auto* elem = polly::get_if<0>(&const_v);
      EXPECT_TRUE((std::is_same<decltype(elem), const int*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, 0);
      {
        auto* bad_elem = polly::get_if<1>(&const_v);
        EXPECT_TRUE(
            (std::is_same<decltype(bad_elem), const std::string*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<2>(&const_v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const double*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<3>(&const_v);
        EXPECT_EQ(bad_elem, nullptr);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const int*>::value));
      }
    }
  }

  {
    Var v = std::string("Hello");
    EXPECT_TRUE(noexcept(polly::get_if<1>(&v)));

    {
      auto* elem = polly::get_if<1>(&v);
      EXPECT_TRUE((std::is_same<decltype(elem), std::string*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, "Hello");
      {
        auto* bad_elem = polly::get_if<0>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<2>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), double*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<3>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
    }

    const Var& const_v = v;
    EXPECT_TRUE(noexcept(polly::get_if<1>(&const_v)));

    {
      auto* elem = polly::get_if<1>(&const_v);
      EXPECT_TRUE((std::is_same<decltype(elem), const std::string*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, "Hello");
      {
        auto* bad_elem = polly::get_if<0>(&const_v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<2>(&const_v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const double*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<3>(&const_v);
        EXPECT_EQ(bad_elem, nullptr);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const int*>::value));
      }
    }
  }

  {
    Var v = 2.0;
    EXPECT_TRUE(noexcept(polly::get_if<2>(&v)));

    {
      auto* elem = polly::get_if<2>(&v);
      EXPECT_TRUE((std::is_same<decltype(elem), double*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, 2.0);
      {
        auto* bad_elem = polly::get_if<0>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<1>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), std::string*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<3>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
    }

    const Var& const_v = v;
    EXPECT_TRUE(noexcept(polly::get_if<2>(&const_v)));

    {
      auto* elem = polly::get_if<2>(&const_v);
      EXPECT_TRUE((std::is_same<decltype(elem), const double*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, 2.0);
      {
        auto* bad_elem = polly::get_if<0>(&const_v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<1>(&const_v);
        EXPECT_TRUE(
            (std::is_same<decltype(bad_elem), const std::string*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<3>(&const_v);
        EXPECT_EQ(bad_elem, nullptr);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const int*>::value));
      }
    }
  }

  {
    Var v(polly::in_place_index<0>, 0);
    v.emplace<3>(1);
    EXPECT_TRUE(noexcept(polly::get_if<3>(&v)));

    {
      auto* elem = polly::get_if<3>(&v);
      EXPECT_TRUE((std::is_same<decltype(elem), int*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, 1);
      {
        auto* bad_elem = polly::get_if<0>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<1>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), std::string*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<2>(&v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), double*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
    }

    const Var& const_v = v;
    EXPECT_TRUE(noexcept(polly::get_if<3>(&const_v)));

    {
      auto* elem = polly::get_if<3>(&const_v);
      EXPECT_TRUE((std::is_same<decltype(elem), const int*>::value));
      ASSERT_NE(elem, nullptr);
      EXPECT_EQ(*elem, 1);
      {
        auto* bad_elem = polly::get_if<0>(&const_v);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const int*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<1>(&const_v);
        EXPECT_TRUE(
            (std::is_same<decltype(bad_elem), const std::string*>::value));
        EXPECT_EQ(bad_elem, nullptr);
      }
      {
        auto* bad_elem = polly::get_if<2>(&const_v);
        EXPECT_EQ(bad_elem, nullptr);
        EXPECT_TRUE((std::is_same<decltype(bad_elem), const double*>::value));
      }
    }
  }
}

//////////////////////
// [variant.relops] //
//////////////////////

TEST(VariantTest, OperatorEquals) {
  variant<int, std::string> a(1), b(1);
  EXPECT_TRUE(a == b);
  EXPECT_TRUE(b == a);
  EXPECT_FALSE(a != b);
  EXPECT_FALSE(b != a);

  b = "str";
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(b == a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(b != a);

  b = 0;
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(b == a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(b != a);

  a = b = "foo";
  EXPECT_TRUE(a == b);
  EXPECT_TRUE(b == a);
  EXPECT_FALSE(a != b);
  EXPECT_FALSE(b != a);

  a = "bar";
  EXPECT_FALSE(a == b);
  EXPECT_FALSE(b == a);
  EXPECT_TRUE(a != b);
  EXPECT_TRUE(b != a);
}

TEST(VariantTest, OperatorRelational) {
  variant<int, std::string> a(1), b(1);
  EXPECT_FALSE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(b > a);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(b <= a);
  EXPECT_TRUE(a >= b);
  EXPECT_TRUE(b >= a);

  b = "str";
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a > b);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(b <= a);
  EXPECT_FALSE(a >= b);
  EXPECT_TRUE(b >= a);

  b = 0;
  EXPECT_FALSE(a < b);
  EXPECT_TRUE(b < a);
  EXPECT_TRUE(a > b);
  EXPECT_FALSE(b > a);
  EXPECT_FALSE(a <= b);
  EXPECT_TRUE(b <= a);
  EXPECT_TRUE(a >= b);
  EXPECT_FALSE(b >= a);

  a = b = "foo";
  EXPECT_FALSE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a > b);
  EXPECT_FALSE(b > a);
  EXPECT_TRUE(a <= b);
  EXPECT_TRUE(b <= a);
  EXPECT_TRUE(a >= b);
  EXPECT_TRUE(b >= a);

  a = "bar";
  EXPECT_TRUE(a < b);
  EXPECT_FALSE(b < a);
  EXPECT_FALSE(a > b);
  EXPECT_TRUE(b > a);
  EXPECT_TRUE(a <= b);
  EXPECT_FALSE(b <= a);
  EXPECT_FALSE(a >= b);
  EXPECT_TRUE(b >= a);
}

#ifdef POLLY_HAVE_EXCEPTIONS

TEST(VariantTest, ValuelessOperatorEquals) {
  variant<MoveCanThrow, std::string> int_v(1), string_v("Hello"),
      valueless(polly::in_place_index<0>),
      other_valueless(polly::in_place_index<0>);
  ToValuelessByException(valueless);
  ToValuelessByException(other_valueless);

  EXPECT_TRUE(valueless == other_valueless);
  EXPECT_TRUE(other_valueless == valueless);
  EXPECT_FALSE(valueless == int_v);
  EXPECT_FALSE(valueless == string_v);
  EXPECT_FALSE(int_v == valueless);
  EXPECT_FALSE(string_v == valueless);

  EXPECT_FALSE(valueless != other_valueless);
  EXPECT_FALSE(other_valueless != valueless);
  EXPECT_TRUE(valueless != int_v);
  EXPECT_TRUE(valueless != string_v);
  EXPECT_TRUE(int_v != valueless);
  EXPECT_TRUE(string_v != valueless);
}

TEST(VariantTest, ValuelessOperatorRelational) {
  variant<MoveCanThrow, std::string> int_v(1), string_v("Hello"),
      valueless(polly::in_place_index<0>),
      other_valueless(polly::in_place_index<0>);
  ToValuelessByException(valueless);
  ToValuelessByException(other_valueless);

  EXPECT_FALSE(valueless < other_valueless);
  EXPECT_FALSE(other_valueless < valueless);
  EXPECT_TRUE(valueless < int_v);
  EXPECT_TRUE(valueless < string_v);
  EXPECT_FALSE(int_v < valueless);
  EXPECT_FALSE(string_v < valueless);

  EXPECT_TRUE(valueless <= other_valueless);
  EXPECT_TRUE(other_valueless <= valueless);
  EXPECT_TRUE(valueless <= int_v);
  EXPECT_TRUE(valueless <= string_v);
  EXPECT_FALSE(int_v <= valueless);
  EXPECT_FALSE(string_v <= valueless);

  EXPECT_TRUE(valueless >= other_valueless);
  EXPECT_TRUE(other_valueless >= valueless);
  EXPECT_FALSE(valueless >= int_v);
  EXPECT_FALSE(valueless >= string_v);
  EXPECT_TRUE(int_v >= valueless);
  EXPECT_TRUE(string_v >= valueless);

  EXPECT_FALSE(valueless > other_valueless);
  EXPECT_FALSE(other_valueless > valueless);
  EXPECT_FALSE(valueless > int_v);
  EXPECT_FALSE(valueless > string_v);
  EXPECT_TRUE(int_v > valueless);
  EXPECT_TRUE(string_v > valueless);
}

#endif

/////////////////////
// [variant.visit] //
/////////////////////

template <typename T>
struct ConvertTo {
  template <typename U>
  T operator()(const U& u) const {
    return u;
  }
};

TEST(VariantTest, VisitSimple) {
  variant<std::string, const char*> v = "A";

  std::string str = polly::visit(ConvertTo<std::string>{}, v);
  EXPECT_EQ("A", str);

  v = std::string("B");

  polly::string_view piece = polly::visit(ConvertTo<polly::string_view>{}, v);
  EXPECT_EQ("B", piece);

  struct StrLen {
    int operator()(const char* s) const { return strlen(s); }
    int operator()(const std::string& s) const { return s.size(); }
  };

  v = "SomeStr";
  EXPECT_EQ(7, polly::visit(StrLen{}, v));
  v = std::string("VeryLargeThisTime");
  EXPECT_EQ(17, polly::visit(StrLen{}, v));
}

TEST(VariantTest, VisitRValue) {
  variant<std::string> v = std::string("X");
  struct Visitor {
    bool operator()(const std::string&) const { return false; }
    bool operator()(std::string&&) const { return true; }  // NOLINT

    int operator()(const std::string&, const std::string&) const { return 0; }
    int operator()(const std::string&, std::string&&) const {
      return 1;
    }  // NOLINT
    int operator()(std::string&&, const std::string&) const {
      return 2;
    }                                                                 // NOLINT
    int operator()(std::string&&, std::string&&) const { return 3; }  // NOLINT
  };
  EXPECT_FALSE(polly::visit(Visitor{}, v));
  EXPECT_TRUE(polly::visit(Visitor{}, std::move(v)));

  // Also test the variadic overload.
  EXPECT_EQ(0, polly::visit(Visitor{}, v, v));
  EXPECT_EQ(1, polly::visit(Visitor{}, v, std::move(v)));
  EXPECT_EQ(2, polly::visit(Visitor{}, std::move(v), v));
  EXPECT_EQ(3, polly::visit(Visitor{}, std::move(v), std::move(v)));
}

TEST(VariantTest, VisitRValueVisitor) {
  variant<std::string> v = std::string("X");
  struct Visitor {
    bool operator()(const std::string&) const& { return false; }
    bool operator()(const std::string&) && { return true; }
  };
  Visitor visitor;
  EXPECT_FALSE(polly::visit(visitor, v));
  EXPECT_TRUE(polly::visit(Visitor{}, v));
}

TEST(VariantTest, VisitResultTypeDifferent) {
  variant<std::string> v = std::string("X");
  struct LValue_LValue {};
  struct RValue_LValue {};
  struct LValue_RValue {};
  struct RValue_RValue {};
  struct Visitor {
    LValue_LValue operator()(const std::string&) const& { return {}; }
    RValue_LValue operator()(std::string&&) const& { return {}; }  // NOLINT
    LValue_RValue operator()(const std::string&) && { return {}; }
    RValue_RValue operator()(std::string&&) && { return {}; }  // NOLINT
  } visitor;

  EXPECT_TRUE(
      (std::is_same<LValue_LValue, decltype(polly::visit(visitor, v))>::value));
  EXPECT_TRUE(
      (std::is_same<RValue_LValue,
                    decltype(polly::visit(visitor, std::move(v)))>::value));
  EXPECT_TRUE((
      std::is_same<LValue_RValue, decltype(polly::visit(Visitor{}, v))>::value));
  EXPECT_TRUE(
      (std::is_same<RValue_RValue,
                    decltype(polly::visit(Visitor{}, std::move(v)))>::value));
}

TEST(VariantTest, VisitVariadic) {
  using A = variant<int, std::string>;
  using B = variant<std::unique_ptr<int>, polly::string_view>;

  struct Visitor {
    std::pair<int, int> operator()(int a, std::unique_ptr<int> b) const {
      return {a, *b};
    }
    std::pair<int, int> operator()(polly::string_view a,
                                   std::unique_ptr<int> b) const {
      return {static_cast<int>(a.size()), static_cast<int>(*b)};
    }
    std::pair<int, int> operator()(int a, polly::string_view b) const {
      return {a, static_cast<int>(b.size())};
    }
    std::pair<int, int> operator()(polly::string_view a,
                                   polly::string_view b) const {
      return {static_cast<int>(a.size()), static_cast<int>(b.size())};
    }
  };

  EXPECT_THAT(polly::visit(Visitor(), A(1), B(std::unique_ptr<int>(new int(7)))),
              ::testing::Pair(1, 7));
  EXPECT_THAT(polly::visit(Visitor(), A(1), B(polly::string_view("ABC"))),
              ::testing::Pair(1, 3));
  EXPECT_THAT(polly::visit(Visitor(), A(std::string("BBBBB")),
                          B(std::unique_ptr<int>(new int(7)))),
              ::testing::Pair(5, 7));
  EXPECT_THAT(polly::visit(Visitor(), A(std::string("BBBBB")),
                          B(polly::string_view("ABC"))),
              ::testing::Pair(5, 3));
}

TEST(VariantTest, VisitNoArgs) {
  EXPECT_EQ(5, polly::visit([] { return 5; }));
}

struct ConstFunctor {
  int operator()(int a, int b) const { return a - b; }
};

struct MutableFunctor {
  int operator()(int a, int b) { return a - b; }
};

struct Class {
  int Method(int a, int b) { return a - b; }
  int ConstMethod(int a, int b) const { return a - b; }

  int member;
};

TEST(VariantTest, VisitReferenceWrapper) {
  ConstFunctor cf;
  MutableFunctor mf;
  polly::variant<int> three = 3;
  polly::variant<int> two = 2;

  EXPECT_EQ(1, polly::visit(std::cref(cf), three, two));
  EXPECT_EQ(1, polly::visit(std::ref(cf), three, two));
  EXPECT_EQ(1, polly::visit(std::ref(mf), three, two));
}

// libstdc++ std::variant doesn't support the INVOKE semantics.
#if !(defined(POLLY_HAVE_STD_VARIANT) && defined(__GLIBCXX__))
TEST(VariantTest, VisitMemberFunction) {
  polly::variant<std::unique_ptr<Class>> p(
      std::unique_ptr<Class>(new Class));
  polly::variant<std::unique_ptr<const Class>> cp(
      std::unique_ptr<const Class>(new Class));
  polly::variant<int> three = 3;
  polly::variant<int> two = 2;

  EXPECT_EQ(1, polly::visit(&Class::Method, p, three, two));
  EXPECT_EQ(1, polly::visit(&Class::ConstMethod, p, three, two));
  EXPECT_EQ(1, polly::visit(&Class::ConstMethod, cp, three, two));
}

TEST(VariantTest, VisitDataMember) {
  polly::variant<std::unique_ptr<Class>> p(std::unique_ptr<Class>(new Class{42}));
  polly::variant<std::unique_ptr<const Class>> cp(
      std::unique_ptr<const Class>(new Class{42}));
  EXPECT_EQ(42, polly::visit(&Class::member, p));

  polly::visit(&Class::member, p) = 5;
  EXPECT_EQ(5, polly::visit(&Class::member, p));

  EXPECT_EQ(42, polly::visit(&Class::member, cp));
}
#endif  // !(defined(POLLY_HAVE_STD_VARIANT) && defined(__GLIBCXX__))

/////////////////////////
// [variant.monostate] //
/////////////////////////

TEST(VariantTest, MonostateBasic) {
  polly::monostate mono;
  (void)mono;

  // TODO(mattcalabrese) Expose move triviality metafunctions in polly.
  EXPECT_TRUE(is_trivially_default_constructible<polly::monostate>::value);
  EXPECT_TRUE(is_trivially_move_constructible<polly::monostate>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<polly::monostate>::value);
  EXPECT_TRUE(is_trivially_move_assignable<polly::monostate>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<polly::monostate>::value);
  EXPECT_TRUE(std::is_trivially_destructible<polly::monostate>::value);
}

TEST(VariantTest, VariantMonostateDefaultConstruction) {
  polly::variant<polly::monostate, NonDefaultConstructible> var;
  EXPECT_EQ(var.index(), 0);
}

////////////////////////////////
// [variant.monostate.relops] //
////////////////////////////////

TEST(VariantTest, MonostateComparisons) {
  polly::monostate lhs, rhs;

  EXPECT_EQ(lhs, lhs);
  EXPECT_EQ(lhs, rhs);

  EXPECT_FALSE(lhs != lhs);
  EXPECT_FALSE(lhs != rhs);
  EXPECT_FALSE(lhs < lhs);
  EXPECT_FALSE(lhs < rhs);
  EXPECT_FALSE(lhs > lhs);
  EXPECT_FALSE(lhs > rhs);

  EXPECT_LE(lhs, lhs);
  EXPECT_LE(lhs, rhs);
  EXPECT_GE(lhs, lhs);
  EXPECT_GE(lhs, rhs);

  EXPECT_TRUE(noexcept(std::declval<polly::monostate>() ==
                       std::declval<polly::monostate>()));
  EXPECT_TRUE(noexcept(std::declval<polly::monostate>() !=
                       std::declval<polly::monostate>()));
  EXPECT_TRUE(noexcept(std::declval<polly::monostate>() <
                       std::declval<polly::monostate>()));
  EXPECT_TRUE(noexcept(std::declval<polly::monostate>() >
                       std::declval<polly::monostate>()));
  EXPECT_TRUE(noexcept(std::declval<polly::monostate>() <=
                       std::declval<polly::monostate>()));
  EXPECT_TRUE(noexcept(std::declval<polly::monostate>() >=
                       std::declval<polly::monostate>()));
}

///////////////////////
// [variant.specalg] //
///////////////////////

TEST(VariantTest, NonmemberSwap) {
  using std::swap;

  SpecialSwap v1(3);
  SpecialSwap v2(7);

  variant<SpecialSwap> a = v1, b = v2;

  EXPECT_THAT(a, VariantWith<SpecialSwap>(v1));
  EXPECT_THAT(b, VariantWith<SpecialSwap>(v2));

  std::swap(a, b);
  EXPECT_THAT(a, VariantWith<SpecialSwap>(v2));
  EXPECT_THAT(b, VariantWith<SpecialSwap>(v1));
#ifndef POLLY_HAVE_STD_VARIANT
  EXPECT_FALSE(polly::get<SpecialSwap>(a).special_swap);
#endif

  swap(a, b);
  EXPECT_THAT(a, VariantWith<SpecialSwap>(v1));
  EXPECT_THAT(b, VariantWith<SpecialSwap>(v2));
  //TODO
  //EXPECT_TRUE(polly::get<SpecialSwap>(b).special_swap);
}

//////////////////////////
// [variant.bad.access] //
//////////////////////////

TEST(VariantTest, BadAccess) {
  EXPECT_TRUE(noexcept(polly::bad_variant_access()));
  polly::bad_variant_access exception_obj;
  std::exception* base = &exception_obj;
  (void)base;
}

////////////////////
// [variant.hash] //
////////////////////

TEST(VariantTest, MonostateHash) {
  polly::monostate mono, other_mono;
  std::hash<polly::monostate> const hasher{};
  static_assert(std::is_same<decltype(hasher(mono)), std::size_t>::value, "");
  EXPECT_EQ(hasher(mono), hasher(other_mono));
}

TEST(VariantTest, Hash) {
  static_assert(polly::is_hashable<variant<int>>::value, "");
  static_assert(polly::is_hashable<variant<Hashable>>::value, "");
  static_assert(polly::is_hashable<variant<int, Hashable>>::value, "");

  {
    std::hash<variant<int>> hash;
    std::set<size_t> hashcodes;
    for (int i = 0; i < 100; ++i) {
      hashcodes.insert(hash(i));
    }
    EXPECT_GT(hashcodes.size(), 90);

    // test const-qualified
    static_assert(polly::is_hashable<variant<const int>>::value, "");
    static_assert(polly::is_hashable<variant<const Hashable>>::value, "");
    std::hash<polly::variant<const int>> c_hash;
    for (int i = 0; i < 100; ++i) {
      EXPECT_EQ(hash(i), c_hash(i));
    }
  }
}

////////////////////////////////////////
// Miscellaneous and deprecated tests //
////////////////////////////////////////

// Test that a set requiring a basic type conversion works correctly
#if !defined(POLLY_HAVE_STD_VARIANT)
TEST(VariantTest, TestConvertingSet) {
  typedef variant<double> Variant;
  Variant v(1.0);
  const int two = 2;
  v = two;
  EXPECT_TRUE(polly::holds_alternative<double>(v));
  ASSERT_TRUE(nullptr != polly::get_if<double>(&v));
  EXPECT_DOUBLE_EQ(2, polly::get<double>(v));
}
#endif  // POLLY_HAVE_STD_VARIANT

// Test that a vector of variants behaves reasonably.
TEST(VariantTest, Container) {
  typedef variant<int, float> Variant;

  // Creation of vector should work
  std::vector<Variant> vec;
  vec.push_back(Variant(10));
  vec.push_back(Variant(20.0f));

  // Vector resizing should work if we supply a value for new slots
  vec.resize(10, Variant(0));
}

// Test that a variant with a non-copyable type can be constructed and
// manipulated to some degree.
TEST(VariantTest, TestVariantWithNonCopyableType) {
  typedef variant<int, NonCopyable> Variant;
  const int kValue = 1;
  Variant v(kValue);
  ASSERT_TRUE(polly::holds_alternative<int>(v));
  EXPECT_EQ(kValue, polly::get<int>(v));
}

// Test that a variant with a non-copyable type can be transformed to
// the non-copyable type with a call to `emplace` for different numbers
// of arguments. We do not need to test this for each of T1 ... T8
// because `emplace` does not overload on T1 ... to T8, so if this
// works for any one of T1 ... T8, then it works for all of them. We
// do need to test that it works with varying numbers of parameters
// though.
TEST(VariantTest, TestEmplace) {
  typedef variant<int, NonCopyable> Variant;
  const int kValue = 1;
  Variant v(kValue);
  ASSERT_TRUE(polly::holds_alternative<int>(v));
  EXPECT_EQ(kValue, polly::get<int>(v));

  // emplace with zero arguments, then back to 'int'
  v.emplace<NonCopyable>();
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v));
  EXPECT_EQ(0, polly::get<NonCopyable>(v).value);
  v = kValue;
  ASSERT_TRUE(polly::holds_alternative<int>(v));

  // emplace with one argument:
  v.emplace<NonCopyable>(1);
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v));
  EXPECT_EQ(1, polly::get<NonCopyable>(v).value);
  v = kValue;
  ASSERT_TRUE(polly::holds_alternative<int>(v));

  // emplace with two arguments:
  v.emplace<NonCopyable>(1, 2);
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v));
  EXPECT_EQ(3, polly::get<NonCopyable>(v).value);
  v = kValue;
  ASSERT_TRUE(polly::holds_alternative<int>(v));

  // emplace with three arguments
  v.emplace<NonCopyable>(1, 2, 3);
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v));
  EXPECT_EQ(6, polly::get<NonCopyable>(v).value);
  v = kValue;
  ASSERT_TRUE(polly::holds_alternative<int>(v));

  // emplace with four arguments
  v.emplace<NonCopyable>(1, 2, 3, 4);
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v));
  EXPECT_EQ(10, polly::get<NonCopyable>(v).value);
  v = kValue;
  ASSERT_TRUE(polly::holds_alternative<int>(v));
}

TEST(VariantTest, TestEmplaceDestroysCurrentValue) {
  typedef variant<int, IncrementInDtor, NonCopyable> Variant;
  int counter = 0;
  Variant v(0);
  ASSERT_TRUE(polly::holds_alternative<int>(v));
  v.emplace<IncrementInDtor>(&counter);
  ASSERT_TRUE(polly::holds_alternative<IncrementInDtor>(v));
  ASSERT_EQ(0, counter);
  v.emplace<NonCopyable>();
  ASSERT_TRUE(polly::holds_alternative<NonCopyable>(v));
  EXPECT_EQ(1, counter);
}

TEST(VariantTest, TestMoveSemantics) {
  typedef variant<std::unique_ptr<int>, std::unique_ptr<std::string>> Variant;

  // Construct a variant by moving from an element value.
  Variant v(std::unique_ptr<int>(new int(10)));
  EXPECT_TRUE(polly::holds_alternative<std::unique_ptr<int>>(v));

  // Construct a variant by moving from another variant.
  Variant v2(std::move(v));
  ASSERT_TRUE(polly::holds_alternative<std::unique_ptr<int>>(v2));
  ASSERT_NE(nullptr, polly::get<std::unique_ptr<int>>(v2));
  EXPECT_EQ(10, *polly::get<std::unique_ptr<int>>(v2));

  // Moving from a variant object leaves it holding moved-from value of the
  // same element type.
  EXPECT_TRUE(polly::holds_alternative<std::unique_ptr<int>>(v));
  ASSERT_NE(nullptr, polly::get_if<std::unique_ptr<int>>(&v));
  EXPECT_EQ(nullptr, polly::get<std::unique_ptr<int>>(v));

  // Assign a variant from an element value by move.
  v = std::unique_ptr<std::string>(new std::string{"foo"});
  ASSERT_TRUE(polly::holds_alternative<std::unique_ptr<std::string>>(v));
  EXPECT_EQ("foo", *polly::get<std::unique_ptr<std::string>>(v));

  // Move-assign a variant.
  v2 = std::move(v);
  ASSERT_TRUE(polly::holds_alternative<std::unique_ptr<std::string>>(v2));
  EXPECT_EQ("foo", *polly::get<std::unique_ptr<std::string>>(v2));
  EXPECT_TRUE(polly::holds_alternative<std::unique_ptr<std::string>>(v));
}

variant<int, std::string> PassThrough(const variant<int, std::string>& arg) {
  return arg;
}

TEST(VariantTest, TestImplicitConversion) {
  EXPECT_TRUE(polly::holds_alternative<int>(PassThrough(0)));

  // We still need the explicit cast for std::string, because C++ won't apply
  // two user-defined implicit conversions in a row.
  EXPECT_TRUE(polly::holds_alternative<std::string>(PassThrough(std::string("foo"))));
}

struct Convertible2;
struct Convertible1 {
  Convertible1() {}
  Convertible1(const Convertible1&) {}
  Convertible1& operator=(const Convertible1&) { return *this; }

  // implicit conversion from Convertible2
  Convertible1(const Convertible2&) {}  // NOLINT(runtime/explicit)
};

struct Convertible2 {
  Convertible2() {}
  Convertible2(const Convertible2&) {}
  Convertible2& operator=(const Convertible2&) { return *this; }

  // implicit conversion from Convertible1
  Convertible2(const Convertible1&) {}  // NOLINT(runtime/explicit)
};

TEST(VariantTest, TestRvalueConversion) {
#if !defined(POLLY_HAVE_STD_VARIANT)
  variant<double, std::string> var(
      ConvertVariantTo<variant<double, std::string>>(
          variant<std::string, int>(0)));
  ASSERT_TRUE(polly::holds_alternative<double>(var));
  EXPECT_EQ(0.0, polly::get<double>(var));

  var = ConvertVariantTo<variant<double, std::string>>(
      variant<const char*, float>("foo"));
  ASSERT_TRUE(polly::holds_alternative<std::string>(var));
  EXPECT_EQ("foo", polly::get<std::string>(var));

  variant<double> singleton(
      ConvertVariantTo<variant<double>>(variant<int, float>(42)));
  ASSERT_TRUE(polly::holds_alternative<double>(singleton));
  EXPECT_EQ(42.0, polly::get<double>(singleton));

  singleton = ConvertVariantTo<variant<double>>(variant<int, float>(3.14f));
  ASSERT_TRUE(polly::holds_alternative<double>(singleton));
  EXPECT_FLOAT_EQ(3.14f, static_cast<float>(polly::get<double>(singleton)));

  singleton = ConvertVariantTo<variant<double>>(variant<int>(0));
  ASSERT_TRUE(polly::holds_alternative<double>(singleton));
  EXPECT_EQ(0.0, polly::get<double>(singleton));

  variant<int32_t, uint32_t> variant2(
      ConvertVariantTo<variant<int32_t, uint32_t>>(variant<int32_t>(42)));
  ASSERT_TRUE(polly::holds_alternative<int32_t>(variant2));
  EXPECT_EQ(42, polly::get<int32_t>(variant2));

  variant2 =
      ConvertVariantTo<variant<int32_t, uint32_t>>(variant<uint32_t>(42));
  ASSERT_TRUE(polly::holds_alternative<uint32_t>(variant2));
  EXPECT_EQ(42, polly::get<uint32_t>(variant2));
#endif  // !POLLY_HAVE_STD_VARIANT

  variant<Convertible1, Convertible2> variant3(
      ConvertVariantTo<variant<Convertible1, Convertible2>>(
          (variant<Convertible2, Convertible1>(Convertible1()))));
  ASSERT_TRUE(polly::holds_alternative<Convertible1>(variant3));

  variant3 = ConvertVariantTo<variant<Convertible1, Convertible2>>(
      variant<Convertible2, Convertible1>(Convertible2()));
  ASSERT_TRUE(polly::holds_alternative<Convertible2>(variant3));
}

TEST(VariantTest, TestLvalueConversion) {
#if !defined(POLLY_HAVE_STD_VARIANT)
  variant<std::string, int> source1 = 0;
  variant<double, std::string> destination(
      ConvertVariantTo<variant<double, std::string>>(source1));
  ASSERT_TRUE(polly::holds_alternative<double>(destination));
  EXPECT_EQ(0.0, polly::get<double>(destination));

  variant<const char*, float> source2 = "foo";
  destination = ConvertVariantTo<variant<double, std::string>>(source2);
  ASSERT_TRUE(polly::holds_alternative<std::string>(destination));
  EXPECT_EQ("foo", polly::get<std::string>(destination));

  variant<int, float> source3(42);
  variant<double> singleton(ConvertVariantTo<variant<double>>(source3));
  ASSERT_TRUE(polly::holds_alternative<double>(singleton));
  EXPECT_EQ(42.0, polly::get<double>(singleton));

  source3 = 3.14f;
  singleton = ConvertVariantTo<variant<double>>(source3);
  ASSERT_TRUE(polly::holds_alternative<double>(singleton));
  EXPECT_FLOAT_EQ(3.14f, static_cast<float>(polly::get<double>(singleton)));

  variant<int> source4(0);
  singleton = ConvertVariantTo<variant<double>>(source4);
  ASSERT_TRUE(polly::holds_alternative<double>(singleton));
  EXPECT_EQ(0.0, polly::get<double>(singleton));

  variant<int32_t> source5(42);
  variant<int32_t, uint32_t> variant2(
      ConvertVariantTo<variant<int32_t, uint32_t>>(source5));
  ASSERT_TRUE(polly::holds_alternative<int32_t>(variant2));
  EXPECT_EQ(42, polly::get<int32_t>(variant2));

  variant<uint32_t> source6(42);
  variant2 = ConvertVariantTo<variant<int32_t, uint32_t>>(source6);
  ASSERT_TRUE(polly::holds_alternative<uint32_t>(variant2));
  EXPECT_EQ(42, polly::get<uint32_t>(variant2));
#endif

  variant<Convertible2, Convertible1> source7((Convertible1()));
  variant<Convertible1, Convertible2> variant3(
      ConvertVariantTo<variant<Convertible1, Convertible2>>(source7));
  ASSERT_TRUE(polly::holds_alternative<Convertible1>(variant3));

  source7 = Convertible2();
  variant3 = ConvertVariantTo<variant<Convertible1, Convertible2>>(source7);
  ASSERT_TRUE(polly::holds_alternative<Convertible2>(variant3));
}

TEST(VariantTest, TestMoveConversion) {
  using Variant =
      variant<std::unique_ptr<const int>, std::unique_ptr<const std::string>>;
  using OtherVariant =
      variant<std::unique_ptr<int>, std::unique_ptr<std::string>>;

  Variant var(ConvertVariantTo<Variant>(OtherVariant{std::unique_ptr<int>(new int(0))}));
  ASSERT_TRUE(polly::holds_alternative<std::unique_ptr<const int>>(var));
  ASSERT_NE(polly::get<std::unique_ptr<const int>>(var), nullptr);
  EXPECT_EQ(0, *polly::get<std::unique_ptr<const int>>(var));

  var = ConvertVariantTo<Variant>(
      OtherVariant(std::unique_ptr<std::string>(new std::string{"foo"})));
  ASSERT_TRUE(polly::holds_alternative<std::unique_ptr<const std::string>>(var));
  EXPECT_EQ("foo", *polly::get<std::unique_ptr<const std::string>>(var));
}

TEST(VariantTest, DoesNotMoveFromLvalues) {
  // We use shared_ptr here because it's both copyable and movable, and
  // a moved-from shared_ptr is guaranteed to be null, so we can detect
  // whether moving or copying has occurred.
  using Variant =
      variant<std::shared_ptr<const int>, std::shared_ptr<const std::string>>;
  using OtherVariant =
      variant<std::shared_ptr<int>, std::shared_ptr<std::string>>;

  Variant v1(std::make_shared<const int>(0));

  // Test copy constructor
  Variant v2(v1);
  EXPECT_EQ(polly::get<std::shared_ptr<const int>>(v1),
            polly::get<std::shared_ptr<const int>>(v2));

  // Test copy-assignment operator
  v1 = std::make_shared<const std::string>("foo");
  v2 = v1;
  EXPECT_EQ(polly::get<std::shared_ptr<const std::string>>(v1),
            polly::get<std::shared_ptr<const std::string>>(v2));

  // Test converting copy constructor
  OtherVariant other(std::make_shared<int>(0));
  Variant v3(ConvertVariantTo<Variant>(other));
  EXPECT_EQ(polly::get<std::shared_ptr<int>>(other),
            polly::get<std::shared_ptr<const int>>(v3));

  other = std::make_shared<std::string>("foo");
  v3 = ConvertVariantTo<Variant>(other);
  EXPECT_EQ(polly::get<std::shared_ptr<std::string>>(other),
            polly::get<std::shared_ptr<const std::string>>(v3));
}

TEST(VariantTest, TestRvalueConversionViaConvertVariantTo) {
#if !defined(POLLY_HAVE_STD_VARIANT)
  variant<double, std::string> var(
      ConvertVariantTo<variant<double, std::string>>(
          variant<std::string, int>(3)));
  EXPECT_THAT(polly::get_if<double>(&var), Pointee(3.0));

  var = ConvertVariantTo<variant<double, std::string>>(
      variant<const char*, float>("foo"));
  EXPECT_THAT(polly::get_if<std::string>(&var), Pointee(std::string("foo")));

  variant<double> singleton(
      ConvertVariantTo<variant<double>>(variant<int, float>(42)));
  EXPECT_THAT(polly::get_if<double>(&singleton), Pointee(42.0));

  singleton = ConvertVariantTo<variant<double>>(variant<int, float>(3.14f));
  EXPECT_THAT(polly::get_if<double>(&singleton), Pointee(DoubleEq(3.14f)));

  singleton = ConvertVariantTo<variant<double>>(variant<int>(3));
  EXPECT_THAT(polly::get_if<double>(&singleton), Pointee(3.0));

  variant<int32_t, uint32_t> variant2(
      ConvertVariantTo<variant<int32_t, uint32_t>>(variant<int32_t>(42)));
  EXPECT_THAT(polly::get_if<int32_t>(&variant2), Pointee(42));

  variant2 =
      ConvertVariantTo<variant<int32_t, uint32_t>>(variant<uint32_t>(42));
  EXPECT_THAT(polly::get_if<uint32_t>(&variant2), Pointee(42));
#endif

  variant<Convertible1, Convertible2> variant3(
      ConvertVariantTo<variant<Convertible1, Convertible2>>(
          (variant<Convertible2, Convertible1>(Convertible1()))));
  ASSERT_TRUE(polly::holds_alternative<Convertible1>(variant3));

  variant3 = ConvertVariantTo<variant<Convertible1, Convertible2>>(
      variant<Convertible2, Convertible1>(Convertible2()));
  ASSERT_TRUE(polly::holds_alternative<Convertible2>(variant3));
}

TEST(VariantTest, TestLvalueConversionViaConvertVariantTo) {
#if !defined(POLLY_HAVE_STD_VARIANT)
  variant<std::string, int> source1 = 3;
  variant<double, std::string> destination(
      ConvertVariantTo<variant<double, std::string>>(source1));
  EXPECT_THAT(polly::get_if<double>(&destination), Pointee(3.0));

  variant<const char*, float> source2 = "foo";
  destination = ConvertVariantTo<variant<double, std::string>>(source2);
  EXPECT_THAT(polly::get_if<std::string>(&destination),
              Pointee(std::string("foo")));

  variant<int, float> source3(42);
  variant<double> singleton(ConvertVariantTo<variant<double>>(source3));
  EXPECT_THAT(polly::get_if<double>(&singleton), Pointee(42.0));

  source3 = 3.14f;
  singleton = ConvertVariantTo<variant<double>>(source3);
  EXPECT_FLOAT_EQ(3.14f, static_cast<float>(polly::get<double>(singleton)));
  EXPECT_THAT(polly::get_if<double>(&singleton), Pointee(DoubleEq(3.14f)));

  variant<int> source4(3);
  singleton = ConvertVariantTo<variant<double>>(source4);
  EXPECT_THAT(polly::get_if<double>(&singleton), Pointee(3.0));

  variant<int32_t> source5(42);
  variant<int32_t, uint32_t> variant2(
      ConvertVariantTo<variant<int32_t, uint32_t>>(source5));
  EXPECT_THAT(polly::get_if<int32_t>(&variant2), Pointee(42));

  variant<uint32_t> source6(42);
  variant2 = ConvertVariantTo<variant<int32_t, uint32_t>>(source6);
  EXPECT_THAT(polly::get_if<uint32_t>(&variant2), Pointee(42));
#endif  // !POLLY_HAVE_STD_VARIANT

  variant<Convertible2, Convertible1> source7((Convertible1()));
  variant<Convertible1, Convertible2> variant3(
      ConvertVariantTo<variant<Convertible1, Convertible2>>(source7));
  ASSERT_TRUE(polly::holds_alternative<Convertible1>(variant3));

  source7 = Convertible2();
  variant3 = ConvertVariantTo<variant<Convertible1, Convertible2>>(source7);
  ASSERT_TRUE(polly::holds_alternative<Convertible2>(variant3));
}

TEST(VariantTest, TestMoveConversionViaConvertVariantTo) {
  using Variant =
      variant<std::unique_ptr<const int>, std::unique_ptr<const std::string>>;
  using OtherVariant =
      variant<std::unique_ptr<int>, std::unique_ptr<std::string>>;

  Variant var(
      ConvertVariantTo<Variant>(OtherVariant{std::unique_ptr<int>( new int(3))}));
  EXPECT_THAT(polly::get_if<std::unique_ptr<const int>>(&var),
              Pointee(Pointee(3)));

  var = ConvertVariantTo<Variant>(
      OtherVariant(std::unique_ptr<std::string>(new std::string{"foo"})));
  EXPECT_THAT(polly::get_if<std::unique_ptr<const std::string>>(&var),
              Pointee(Pointee(std::string("foo"))));
}

// If all alternatives are trivially copy/move constructible, variant should
// also be trivially copy/move constructible. This is not required by the
// standard and we know that libstdc++ variant doesn't have this feature.
// For more details see the paper:
// http://open-std.org/JTC1/SC22/WG21/docs/papers/2017/p0602r0.html
#if !(defined(POLLY_HAVE_STD_VARIANT) && defined(__GLIBCXX__))
#define POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY 1
#endif

TEST(VariantTest, TestCopyAndMoveTypeTraits) {
  EXPECT_TRUE(std::is_copy_constructible<variant<std::string>>::value);
  EXPECT_TRUE(std::is_copy_assignable<variant<std::string>>::value);
  EXPECT_TRUE(std::is_move_constructible<variant<std::string>>::value);
  EXPECT_TRUE(std::is_move_assignable<variant<std::string>>::value);
  EXPECT_TRUE(std::is_move_constructible<variant<std::unique_ptr<int>>>::value);
  EXPECT_TRUE(std::is_move_assignable<variant<std::unique_ptr<int>>>::value);
  EXPECT_FALSE(std::is_copy_constructible<variant<std::unique_ptr<int>>>::value);
  EXPECT_FALSE(std::is_copy_assignable<variant<std::unique_ptr<int>>>::value);

  EXPECT_FALSE(polly::is_trivially_copy_constructible<variant<std::string>>::value);
  EXPECT_FALSE(polly::is_trivially_copy_assignable<variant<std::string>>::value);
#if POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY
  EXPECT_TRUE(polly::is_trivially_copy_constructible<variant<int>>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<variant<int>>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<variant<int>>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<variant<int>>::value);
#endif  // POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY
}

TEST(VariantTest, TestVectorOfMoveonlyVariant) {
  // Verify that variant<MoveonlyType> works correctly as a std::vector element.
  std::vector<variant<std::unique_ptr<int>, std::string>> vec;
  vec.push_back(std::unique_ptr<int>(new int(42)));
  vec.emplace_back("Hello");
  vec.reserve(3);
  auto another_vec = std::move(vec);
  // As a sanity check, verify vector contents.
  ASSERT_EQ(2, another_vec.size());
  EXPECT_EQ(42, *polly::get<std::unique_ptr<int>>(another_vec[0]));
  EXPECT_EQ("Hello", polly::get<std::string>(another_vec[1]));
}

TEST(VariantTest, NestedVariant) {
#if POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY
  static_assert(polly::is_trivially_copy_constructible<variant<int>>(), "");
  static_assert(polly::is_trivially_copy_assignable<variant<int>>(), "");
  static_assert(is_trivially_move_constructible<variant<int>>(), "");
  static_assert(is_trivially_move_assignable<variant<int>>(), "");

  static_assert(polly::is_trivially_copy_constructible<variant<variant<int>>>(),
                "");
  static_assert(polly::is_trivially_copy_assignable<variant<variant<int>>>(),
                "");
  static_assert(is_trivially_move_constructible<variant<variant<int>>>(), "");
  static_assert(is_trivially_move_assignable<variant<variant<int>>>(), "");
#endif  // POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY

  variant<int> x(42);
  variant<variant<int>> y(x);
  variant<variant<int>> z(y);
  EXPECT_TRUE(polly::holds_alternative<variant<int>>(z));
  EXPECT_EQ(x, polly::get<variant<int>>(z));
}

struct TriviallyDestructible {
  TriviallyDestructible(TriviallyDestructible&&) {}
  TriviallyDestructible(const TriviallyDestructible&) {}
  TriviallyDestructible& operator=(TriviallyDestructible&&) { return *this; }
  TriviallyDestructible& operator=(const TriviallyDestructible&) {
    return *this;
  }
};

struct TriviallyMovable {
  TriviallyMovable(TriviallyMovable&&) = default;
  TriviallyMovable(TriviallyMovable const&) {}
  TriviallyMovable& operator=(const TriviallyMovable&) { return *this; }
};

struct TriviallyCopyable {
  TriviallyCopyable(const TriviallyCopyable&) = default;
  TriviallyCopyable& operator=(const TriviallyCopyable&) { return *this; }
};

struct TriviallyMoveAssignable {
  TriviallyMoveAssignable(TriviallyMoveAssignable&&) = default;
  TriviallyMoveAssignable(const TriviallyMoveAssignable&) {}
  TriviallyMoveAssignable& operator=(TriviallyMoveAssignable&&) = default;
  TriviallyMoveAssignable& operator=(const TriviallyMoveAssignable&) {
    return *this;
  }
};

struct TriviallyCopyAssignable {};

#if POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY
TEST(VariantTest, TestTriviality) {
  {
    using TrivDestVar = polly::variant<TriviallyDestructible>;

    EXPECT_FALSE(is_trivially_move_constructible<TrivDestVar>::value);
    EXPECT_FALSE(polly::is_trivially_copy_constructible<TrivDestVar>::value);
    EXPECT_FALSE(is_trivially_move_assignable<TrivDestVar>::value);
    EXPECT_FALSE(polly::is_trivially_copy_assignable<TrivDestVar>::value);
    EXPECT_TRUE(std::is_trivially_destructible<TrivDestVar>::value);
  }

  {
    using TrivMoveVar = polly::variant<TriviallyMovable>;

    EXPECT_TRUE(is_trivially_move_constructible<TrivMoveVar>::value);
    EXPECT_FALSE(polly::is_trivially_copy_constructible<TrivMoveVar>::value);
    EXPECT_FALSE(is_trivially_move_assignable<TrivMoveVar>::value);
    EXPECT_FALSE(polly::is_trivially_copy_assignable<TrivMoveVar>::value);
    EXPECT_TRUE(std::is_trivially_destructible<TrivMoveVar>::value);
  }

  {
    using TrivCopyVar = polly::variant<TriviallyCopyable>;

    EXPECT_TRUE(is_trivially_move_constructible<TrivCopyVar>::value);
    EXPECT_TRUE(polly::is_trivially_copy_constructible<TrivCopyVar>::value);
    EXPECT_FALSE(is_trivially_move_assignable<TrivCopyVar>::value);
    EXPECT_FALSE(polly::is_trivially_copy_assignable<TrivCopyVar>::value);
    EXPECT_TRUE(std::is_trivially_destructible<TrivCopyVar>::value);
  }

  {
    using TrivMoveAssignVar = polly::variant<TriviallyMoveAssignable>;

    EXPECT_TRUE(is_trivially_move_constructible<TrivMoveAssignVar>::value);
    EXPECT_FALSE(
        polly::is_trivially_copy_constructible<TrivMoveAssignVar>::value);
    EXPECT_TRUE(is_trivially_move_assignable<TrivMoveAssignVar>::value);
    EXPECT_FALSE(polly::is_trivially_copy_assignable<TrivMoveAssignVar>::value);
    EXPECT_TRUE(std::is_trivially_destructible<TrivMoveAssignVar>::value);
  }

  {
    using TrivCopyAssignVar = polly::variant<TriviallyCopyAssignable>;

    EXPECT_TRUE(is_trivially_move_constructible<TrivCopyAssignVar>::value);
    EXPECT_TRUE(
        polly::is_trivially_copy_constructible<TrivCopyAssignVar>::value);
    EXPECT_TRUE(is_trivially_move_assignable<TrivCopyAssignVar>::value);
    EXPECT_TRUE(polly::is_trivially_copy_assignable<TrivCopyAssignVar>::value);
    EXPECT_TRUE(std::is_trivially_destructible<TrivCopyAssignVar>::value);
  }
}
#endif  // POLLY_VARIANT_PROPAGATE_COPY_MOVE_TRIVIALITY

// To verify that polly::variant correctly use the nontrivial move ctor of its
// member rather than use the trivial copy constructor.
TEST(VariantTest, MoveCtorBug) {
  // To simulate std::tuple in libstdc++.
  struct TrivialCopyNontrivialMove {
    TrivialCopyNontrivialMove() = default;
    TrivialCopyNontrivialMove(const TrivialCopyNontrivialMove&) = default;
    TrivialCopyNontrivialMove(TrivialCopyNontrivialMove&&) { called = true; }
    bool called = false;
  };
  {
    using V = polly::variant<TrivialCopyNontrivialMove, int>;
    V v1(polly::in_place_index<0>);
    // this should invoke the move ctor, rather than the trivial copy ctor.
    V v2(std::move(v1));
    EXPECT_TRUE(polly::get<0>(v2).called);
  }
  {
    // this case failed to compile before our fix due to a GCC bug.
    using V = polly::variant<int, TrivialCopyNontrivialMove>;
    V v1(polly::in_place_index<1>);
    // this should invoke the move ctor, rather than the trivial copy ctor.
    V v2(std::move(v1));
    EXPECT_TRUE(polly::get<1>(v2).called);
  }
}

}  // namespace
}  // namespace polly

#endif  // #if !defined(POLLY_HAVE_STD_VARIANT)
