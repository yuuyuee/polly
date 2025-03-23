// Copyright 2022 The Oak Authors.

#include <vector>
#include "oak/common/any.h"
#include "gtest/gtest.h"

using oak::Any;
using oak::any_cast;
using oak::in_place_type_t;
using oak::in_place_type;
using oak::is_in_place_type;

namespace {

template <typename Tp>
const Tp& AsConst(const Tp& v) { return v; }

struct MoveOnly {
  MoveOnly() = default;
  explicit MoveOnly(int value) : value(value) {}
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;

  int value = 0;
};

struct CopyOnly {
  CopyOnly() = default;
  explicit CopyOnly(int value) : value(value) {}
  CopyOnly(CopyOnly&&) = delete;
  CopyOnly& operator=(CopyOnly&&) = delete;
  CopyOnly(const CopyOnly&) = default;
  CopyOnly& operator=(const CopyOnly&) = default;

  int value = 0;
};

struct MoveOnlyWithListConstructor {
  MoveOnlyWithListConstructor() = default;
  explicit MoveOnlyWithListConstructor(std::initializer_list<int>, int value)
      : value(value) {}
  MoveOnlyWithListConstructor(MoveOnlyWithListConstructor&&) = default;
  MoveOnlyWithListConstructor& operator=(
      MoveOnlyWithListConstructor&&) = default;

  int value = 0;
};

struct IntMoveOnlyCopyOnly {
  IntMoveOnlyCopyOnly(int value, MoveOnly, CopyOnly): value(value) {}

  int value;
};

struct ListMoveOnlyCopyOnly {
  ListMoveOnlyCopyOnly(std::initializer_list<int> ilist, MoveOnly,  CopyOnly)
      : values(ilist) {}

  std::vector<int> values;
};

using FunctionType = void();
void FunctionToEmplace() {}

using ArrayType = int[2];
using DecayedArray = typename std::decay<ArrayType>::type;

TEST(AnyTest, Noexcept) {
  static_assert(std::is_nothrow_default_constructible<Any>(), "");
  static_assert(std::is_nothrow_move_constructible<Any>(), "");
  static_assert(std::is_nothrow_move_assignable<Any>(), "");
  static_assert(noexcept(std::declval<Any&>().has_value()), "");
#if defined(__GXX_RTTI)
  static_assert(noexcept(std::declval<Any&>().type()), "");
#endif
  static_assert(noexcept(any_cast<int>(std::declval<Any*>())), "");
  static_assert(noexcept(std::declval<Any&>().swap(std::declval<Any&>())), "");

  using std::swap;
  static_assert(noexcept(swap(std::declval<Any&>(), std::declval<Any&>())), "");
}

TEST(AnyTest, HasValue) {
  Any o;
  EXPECT_FALSE(o.has_value());
  o.emplace<int>();
  EXPECT_TRUE(o.has_value());
  o.reset();
  EXPECT_FALSE(o.has_value());
}

#if defined(__GXX_RTTI)
TEST(AnyTest, Type) {
  Any o;
  EXPECT_EQ(typeid(void), o.type());
  o.emplace<int>(5);
  EXPECT_EQ(typeid(int), o.type());
  o.emplace<float>(5.f);
  EXPECT_EQ(typeid(float), o.type());
  o.reset();
  EXPECT_EQ(typeid(void), o.type());
}
#endif

TEST(AnyTest, EmptyPointerCast) {
  // pointer-to-unqualified overload
  {
    Any o;
    EXPECT_EQ(nullptr, any_cast<int>(&o));
    o.emplace<int>();
    EXPECT_NE(nullptr, any_cast<int>(&o));
    o.reset();
    EXPECT_EQ(nullptr, any_cast<int>(&o));
  }

  // pointer-to-const overload
  {
    Any o;
    EXPECT_EQ(nullptr, any_cast<int>(&AsConst(o)));
    o.emplace<int>();
    EXPECT_NE(nullptr, any_cast<int>(&AsConst(o)));
    o.reset();
    EXPECT_EQ(nullptr, any_cast<int>(&AsConst(o)));
  }
}

TEST(AnyTest, InPlaceConstruction) {
  const CopyOnly copy_only{};
  Any o(in_place_type_t<IntMoveOnlyCopyOnly>(), 5, MoveOnly(),
              copy_only);
  IntMoveOnlyCopyOnly& v = any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionVariableTemplate) {
  const CopyOnly copy_only{};
  Any o(in_place_type<IntMoveOnlyCopyOnly>, 5, MoveOnly(), copy_only);
  auto& v = any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionWithCV) {
  const CopyOnly copy_only{};
  Any o(in_place_type_t<const volatile IntMoveOnlyCopyOnly>(),
                        5, MoveOnly(), copy_only);
  IntMoveOnlyCopyOnly& v = any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionWithCVVariableTemplate) {
  const CopyOnly copy_only{};
  Any o(in_place_type<const volatile IntMoveOnlyCopyOnly>,
                      5, MoveOnly(), copy_only);
  auto& v = any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionWithFunction) {
  Any o(in_place_type_t<FunctionType>(), FunctionToEmplace);
  FunctionType*& construction_result = any_cast<FunctionType*&>(o);
  EXPECT_EQ(&FunctionToEmplace, construction_result);
}

TEST(AnyTest, InPlaceConstructionWithFunctionVariableTemplate) {
  Any o(in_place_type<FunctionType>, FunctionToEmplace);
  auto& construction_result = any_cast<FunctionType*&>(o);
  EXPECT_EQ(&FunctionToEmplace, construction_result);
}

TEST(AnyTest, InPlaceConstructionWithArray) {
  ArrayType ar = {5, 42};
  Any o(in_place_type_t<ArrayType>(), ar);
  DecayedArray& construction_result = any_cast<DecayedArray&>(o);
  EXPECT_EQ(&ar[0], construction_result);
}

TEST(AnyTest, InPlaceConstructionWithArrayVariableTemplate) {
  ArrayType ar = {5, 42};
  Any o(in_place_type<ArrayType>, ar);
  auto& construction_result = any_cast<DecayedArray&>(o);
  EXPECT_EQ(&ar[0], construction_result);
}

TEST(AnyTest, InPlaceConstructionIlist) {
  const CopyOnly copy_only{};
  Any o(in_place_type_t<ListMoveOnlyCopyOnly>(),
                        {1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceConstructionIlistVariableTemplate) {
  const CopyOnly copy_only{};
  Any o(in_place_type<ListMoveOnlyCopyOnly>,
                      {1, 2, 3, 4}, MoveOnly(), copy_only);
  auto& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceConstructionIlistWithCV) {
  const CopyOnly copy_only{};
  Any o(in_place_type_t<const volatile ListMoveOnlyCopyOnly>(),
              {1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceConstructionIlistWithCVVariableTemplate) {
  const CopyOnly copy_only{};
  Any o(in_place_type<const volatile ListMoveOnlyCopyOnly>,
                      {1, 2, 3, 4}, MoveOnly(), copy_only);
  auto& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceNoArgs) {
  Any o(in_place_type_t<int>{});
  EXPECT_EQ(0, any_cast<int&>(o));
}

TEST(AnyTest, InPlaceNoArgsVariableTemplate) {
  Any o(in_place_type<int>);
  EXPECT_EQ(0, any_cast<int&>(o));
}

template <typename...>
struct Void {
  using type = void;
};

template <typename... Args>
using void_t = typename Void<Args...>::type;

template <typename Enabler, typename T, typename... Args>
struct CanEmplaceAnyImpl : std::false_type {};

template <typename T, typename... Args>
struct CanEmplaceAnyImpl<
    void_t<decltype(
        std::declval<Any&>().emplace<T>(std::declval<Args>()...))>,
    T, Args...> : std::true_type {};

template <typename T, typename... Args>
using CanEmplaceAny = CanEmplaceAnyImpl<void, T, Args...>;

TEST(AnyTest, Emplace) {
  const CopyOnly copy_only{};
  Any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<IntMoveOnlyCopyOnly>(
                                5, MoveOnly(), copy_only)),
                            IntMoveOnlyCopyOnly&>::value));
  IntMoveOnlyCopyOnly& emplace_result =
      o.emplace<IntMoveOnlyCopyOnly>(5, MoveOnly(), copy_only);
  EXPECT_EQ(5, emplace_result.value);
  IntMoveOnlyCopyOnly& v = any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
  EXPECT_EQ(&emplace_result, &v);

  static_assert(!CanEmplaceAny<int, int, int>::value, "");
  static_assert(!CanEmplaceAny<MoveOnly, MoveOnly>::value, "");
}

TEST(AnyTest, EmplaceWithCV) {
  const CopyOnly copy_only{};
  Any o;
  EXPECT_TRUE((
      std::is_same<
          decltype(
              o.emplace<const volatile IntMoveOnlyCopyOnly>(
                  5, MoveOnly(), copy_only)), IntMoveOnlyCopyOnly&>::value));
  IntMoveOnlyCopyOnly& emplace_result =
      o.emplace<const volatile IntMoveOnlyCopyOnly>(5, MoveOnly(), copy_only);
  EXPECT_EQ(5, emplace_result.value);
  IntMoveOnlyCopyOnly& v = any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
  EXPECT_EQ(&emplace_result, &v);
}

TEST(AnyTest, EmplaceWithFunction) {
  Any o;
  EXPECT_TRUE(
      (std::is_same<decltype(o.emplace<FunctionType>(FunctionToEmplace)),
          FunctionType*&>::value));
  FunctionType*& emplace_result = o.emplace<FunctionType>(FunctionToEmplace);
  EXPECT_EQ(&FunctionToEmplace, emplace_result);
}

TEST(AnyTest, EmplaceWithArray) {
  Any o;
  ArrayType ar = {5, 42};
  EXPECT_TRUE(
      (std::is_same<decltype(o.emplace<ArrayType>(ar)),
                             DecayedArray&>::value));
  DecayedArray& emplace_result = o.emplace<ArrayType>(ar);
  EXPECT_EQ(&ar[0], emplace_result);
}

TEST(AnyTest, EmplaceIlist) {
  const CopyOnly copy_only{};
  Any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<ListMoveOnlyCopyOnly>(
      {1, 2, 3, 4}, MoveOnly(), copy_only)), ListMoveOnlyCopyOnly&>::value));
  ListMoveOnlyCopyOnly& emplace_result =
      o.emplace<ListMoveOnlyCopyOnly>({1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(&v, &emplace_result);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);

  static_assert(!CanEmplaceAny<int, std::initializer_list<int>>::value, "");
  static_assert(!CanEmplaceAny<MoveOnlyWithListConstructor,
                               std::initializer_list<int>, int>::value, "");
}

TEST(AnyTest, EmplaceIlistWithCV) {
  const CopyOnly copy_only{};
  Any o;
  EXPECT_TRUE((
      std::is_same<decltype(o.emplace<const volatile ListMoveOnlyCopyOnly>(
          {1, 2, 3, 4}, MoveOnly(), copy_only))
          , ListMoveOnlyCopyOnly&>::value));
  ListMoveOnlyCopyOnly& emplace_result =
      o.emplace<const volatile ListMoveOnlyCopyOnly>(
          {1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(&v, &emplace_result);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, EmplaceNoArgs) {
  Any o;
  o.emplace<int>();
  EXPECT_EQ(0, any_cast<int>(o));
}

TEST(AnyTest, Swap) {
  Any f0(100);
  Any f1(200);

  EXPECT_EQ(any_cast<int>(f0), 100);
  EXPECT_EQ(any_cast<int>(f1), 200);
  using std::swap;
  swap(f0, f1);
  EXPECT_EQ(any_cast<int>(f0), 200);
  EXPECT_EQ(any_cast<int>(f1), 100);
}

TEST(AnyTest, ConversionConstruction) {
  {
    Any o = 5;
    EXPECT_EQ(5, any_cast<int>(o));
  }

  {
    const CopyOnly copy_only(5);
    Any o = copy_only;
    EXPECT_EQ(5, any_cast<CopyOnly&>(o).value);
  }

  static_assert(!std::is_convertible<MoveOnly, Any>::value, "");
}

TEST(AnyTest, ConversionAssignment) {
  {
    Any o;
    o = 5;
    EXPECT_EQ(5, any_cast<int>(o));
  }

  {
    const CopyOnly copy_only(5);
    Any o;
    o = copy_only;
    EXPECT_EQ(5, any_cast<CopyOnly&>(o).value);
  }

  static_assert(!std::is_assignable<MoveOnly, Any>::value, "");
}

struct WeirdConstructor42 {
  explicit WeirdConstructor42(int value) : value(value) {}

  // Copy-constructor
  WeirdConstructor42(const WeirdConstructor42& other) : value(other.value) {}

  // L-value "weird" constructor (used when given an l-value)
  WeirdConstructor42(WeirdConstructor42&) : value(42) {}

  int value;
};

TEST(AnyTest, WeirdConversionConstruction) {
  {
    const WeirdConstructor42 source(5);
    Any o = source;  // Actual copy
    EXPECT_EQ(5, any_cast<WeirdConstructor42&>(o).value);
  }

  {
    WeirdConstructor42 source(5);
    Any o = source;  // Weird "conversion"
    EXPECT_EQ(42, any_cast<WeirdConstructor42&>(o).value);
  }
}

TEST(AnyTest, WeirdConversionAssignment) {
  {
    const WeirdConstructor42 source(5);
    Any o;
    o = source;  // Actual copy
    EXPECT_EQ(5, any_cast<WeirdConstructor42&>(o).value);
  }

  {
    WeirdConstructor42 source(5);
    Any o;
    o = source;  // Weird "conversion"
    EXPECT_EQ(42, any_cast<WeirdConstructor42&>(o).value);
  }
}

struct Value {};

TEST(AnyTest, AnyCastValue) {
  {
    Any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, any_cast<int>(o));
    EXPECT_EQ(5, any_cast<int>(AsConst(o)));
    static_assert(std::is_same<decltype(any_cast<Value>(o)), Value>::value, "");
  }

  {
    Any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, any_cast<const int>(o));
    EXPECT_EQ(5, any_cast<const int>(AsConst(o)));
    static_assert(std::is_same<decltype(any_cast<const Value>(o)),
                  const Value>::value,  "");
  }
}

TEST(AnyTest, AnyCastReference) {
  {
    Any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, any_cast<int&>(o));
    EXPECT_EQ(5, any_cast<const int&>(AsConst(o)));
    static_assert(std::is_same<decltype(any_cast<Value&>(o)),
                  Value&>::value, "");
  }

  {
    Any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, any_cast<const int>(o));
    EXPECT_EQ(5, any_cast<const int>(AsConst(o)));
    static_assert(std::is_same<decltype(any_cast<const Value&>(o)),
                  const Value&>::value, "");
  }

  {
    Any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, any_cast<int&&>(std::move(o)));
    static_assert(std::is_same<decltype(any_cast<Value&&>(std::move(o))),
                  Value&&>::value, "");
  }

  {
    Any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, any_cast<const int>(std::move(o)));
    static_assert(std::is_same<decltype(any_cast<const Value&&>(std::move(o))),
                  const Value&&>::value, "");
  }
}

TEST(AnyTest, AnyCastPointer) {
  {
    Any o;
    EXPECT_EQ(nullptr, any_cast<char>(&o));
    o.emplace<int>(5);
    EXPECT_EQ(nullptr, any_cast<char>(&o));
    o.emplace<char>('a');
    EXPECT_EQ('a', *any_cast<char>(&o));
    static_assert(std::is_same<decltype(any_cast<Value>(&o)),
                  Value*>::value, "");
  }

  {
    Any o;
    EXPECT_EQ(nullptr, any_cast<const char>(&o));
    o.emplace<int>(5);
    EXPECT_EQ(nullptr, any_cast<const char>(&o));
    o.emplace<char>('a');
    EXPECT_EQ('a', *any_cast<const char>(&o));
    static_assert(std::is_same<decltype(any_cast<const Value>(&o)),
                  const Value*>::value, "");
  }
}

TEST(AnyTest, MakeAny) {
  const CopyOnly copy_only{};
  auto o = oak::make_any<IntMoveOnlyCopyOnly>(5, MoveOnly(), copy_only);
  static_assert(std::is_same<decltype(o), Any>::value, "");
  EXPECT_EQ(5, any_cast<IntMoveOnlyCopyOnly&>(o).value);
}

TEST(AnyTest, MakeAnyIList) {
  const CopyOnly copy_only{};
  auto o =
      oak::make_any<ListMoveOnlyCopyOnly>({1, 2, 3}, MoveOnly(), copy_only);
  static_assert(std::is_same<decltype(o), Any>::value, "");
  ListMoveOnlyCopyOnly& v = any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values{1, 2, 3};
  EXPECT_EQ(expected_values, v.values);
}

// Test the use of copy constructor and operator=
TEST(AnyTest, Copy) {
  {
    Any o(in_place_type<int>, 123);
    int* f1 = any_cast<int>(&o);

    Any o2(o);
    const int* f2 = any_cast<int>(&o2);
    EXPECT_EQ(123, *f2);
    EXPECT_NE(f1, f2);

    Any o3;
    o3 = o2;
    const int* f3 = any_cast<int>(&o3);
    EXPECT_EQ(123, *f3);
    EXPECT_NE(f2, f3);

    const Any o4(4);
    // copy construct from const lvalue ref.
    Any o5 = o4;
    EXPECT_EQ(4, any_cast<int>(o4));
    EXPECT_EQ(4, any_cast<int>(o5));

    // Copy construct from const rvalue ref.
    Any o6 = std::move(o4);  // NOLINT
    EXPECT_EQ(4, any_cast<int>(o4));
    EXPECT_EQ(4, any_cast<int>(o6));
  }
}

TEST(AnyTest, Move) {
  Any tmp4(4);
  Any o4(std::move(tmp4));  // move construct
  EXPECT_EQ(4, any_cast<int>(o4));
  o4 = *&o4;  // self assign
  EXPECT_EQ(4, any_cast<int>(o4));
  EXPECT_TRUE(o4.has_value());

  Any o5;
  Any tmp5(5);
  o5 = std::move(tmp5);  // move assign
  EXPECT_EQ(5, any_cast<int>(o5));
}

// Reset the ObjectOwner with an object of a different type
TEST(AnyTest, Reset) {
  Any o;
  o.emplace<int>();

  o.reset();
  EXPECT_FALSE(o.has_value());

  o.emplace<char>();
  EXPECT_TRUE(o.has_value());
}

// If using the `Any` implementation, we can rely on a specific message.
#define OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(expr) do {   \
  bool flags = false;                                 \
  try { expr; } catch (oak::bad_any_cast& e) {        \
    flags = true;                                     \
  }                                                   \
  EXPECT_TRUE(flags);                                 \
} while (0)

TEST(AnyTest, ThrowBadAlloc) {
  {
    Any a;
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<int&>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const int&>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<int&&>(Any{}));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const int&&>(Any{}));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<int>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const int>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<int>(Any{}));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const int>(Any{}));

    // const Any operand
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const int&>(AsConst(a)));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<int>(AsConst(a)));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const int>(AsConst(a)));
  }

  {
    Any a(in_place_type<int>);
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<float&>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const float&>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<float&&>(Any{}));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const float&&>(Any{}));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<float>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const float>(a));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<float>(Any{}));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const float>(Any{}));

    // const Any operand
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const float&>(AsConst(a)));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<float>(AsConst(a)));
    OAK_ANY_TEST_EXPECT_BAD_ANY_CAST(any_cast<const float>(AsConst(a)));
  }
}

}  // anonymous namespace
