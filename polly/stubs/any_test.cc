#include "stubs/internal/config.h"

#ifndef POLLY_HAVE_STD_OPTIONAL
#include <vector>
#include <initializer_list>

#include "stubs/type_traits.h"
#include "stubs/utility.h"
#include "stubs/any.h"
#include "gtest/gtest.h"

namespace {
//using polly::test_internal::CopyableOnlyInstance;
//using polly::test_internal::InstanceTracker;

template <typename Tp> const Tp& AsConst(const Tp& v) { return v; }

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
  MoveOnlyWithListConstructor& operator=(MoveOnlyWithListConstructor&&) = default;

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
  static_assert(std::is_nothrow_default_constructible<polly::any>(), "");
  static_assert(std::is_nothrow_move_constructible<polly::any>(), "");
  static_assert(std::is_nothrow_move_assignable<polly::any>(), "");
  static_assert(noexcept(std::declval<polly::any&>().has_value()), "");
#ifdef POLLY_HAVE_RTTI
  static_assert(noexcept(std::declval<polly::any&>().type()), "");
#endif // POLLY_HAVE_RTTI
  static_assert(noexcept(polly::any_cast<int>(std::declval<polly::any*>())), "");
  static_assert(noexcept(std::declval<polly::any&>().swap(std::declval<polly::any&>())), "");

  using std::swap;
  static_assert(noexcept(swap(std::declval<polly::any&>(), std::declval<polly::any&>())), "");
}

TEST(AnyTest, HasValue) {
  polly::any o;
  EXPECT_FALSE(o.has_value());
  o.emplace<int>();
  EXPECT_TRUE(o.has_value());
  o.reset();
  EXPECT_FALSE(o.has_value());
}

#ifdef POLLY_HAVE_RTTI
TEST(AnyTest, Type) {
  polly::any o;
  EXPECT_EQ(typeid(void), o.type());
  o.emplace<int>(5);
  EXPECT_EQ(typeid(int), o.type());
  o.emplace<float>(5.f);
  EXPECT_EQ(typeid(float), o.type());
  o.reset();
  EXPECT_EQ(typeid(void), o.type());
}
#endif // POLLY_HAVE_RTTI

TEST(AnyTest, EmptyPointerCast) {
  // pointer-to-unqualified overload
  {
    polly::any o;
    EXPECT_EQ(nullptr, polly::any_cast<int>(&o));
    o.emplace<int>();
    EXPECT_NE(nullptr, polly::any_cast<int>(&o));
    o.reset();
    EXPECT_EQ(nullptr, polly::any_cast<int>(&o));
  }

  // pointer-to-const overload
  {
    polly::any o;
    EXPECT_EQ(nullptr, polly::any_cast<int>(&AsConst(o)));
    o.emplace<int>();
    EXPECT_NE(nullptr, polly::any_cast<int>(&AsConst(o)));
    o.reset();
    EXPECT_EQ(nullptr, polly::any_cast<int>(&AsConst(o)));
  }
}

TEST(AnyTest, InPlaceConstruction) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type_t<IntMoveOnlyCopyOnly>(), 5, MoveOnly(),
              copy_only);
  IntMoveOnlyCopyOnly& v = polly::any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionVariableTemplate) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type<IntMoveOnlyCopyOnly>, 5, MoveOnly(), copy_only);
  auto& v = polly::any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionWithCV) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type_t<const volatile IntMoveOnlyCopyOnly>(), 5, MoveOnly(), copy_only);
  IntMoveOnlyCopyOnly& v = polly::any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionWithCVVariableTemplate) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type<const volatile IntMoveOnlyCopyOnly>, 5, MoveOnly(), copy_only);
  auto& v = polly::any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
}

TEST(AnyTest, InPlaceConstructionWithFunction) {
  polly::any o(polly::in_place_type_t<FunctionType>(), FunctionToEmplace);
  FunctionType*& construction_result = polly::any_cast<FunctionType*&>(o);
  EXPECT_EQ(&FunctionToEmplace, construction_result);
}

TEST(AnyTest, InPlaceConstructionWithFunctionVariableTemplate) {
  polly::any o(polly::in_place_type<FunctionType>, FunctionToEmplace);
  auto& construction_result = polly::any_cast<FunctionType*&>(o);
  EXPECT_EQ(&FunctionToEmplace, construction_result);
}

TEST(AnyTest, InPlaceConstructionWithArray) {
  ArrayType ar = {5, 42};
  polly::any o(polly::in_place_type_t<ArrayType>(), ar);
  DecayedArray& construction_result = polly::any_cast<DecayedArray&>(o);
  EXPECT_EQ(&ar[0], construction_result);
}

TEST(AnyTest, InPlaceConstructionWithArrayVariableTemplate) {
  ArrayType ar = {5, 42};
  polly::any o(polly::in_place_type<ArrayType>, ar);
  auto& construction_result = polly::any_cast<DecayedArray&>(o);
  EXPECT_EQ(&ar[0], construction_result);
}

TEST(AnyTest, InPlaceConstructionIlist) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type_t<ListMoveOnlyCopyOnly>(), {1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceConstructionIlistVariableTemplate) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type<ListMoveOnlyCopyOnly>, {1, 2, 3, 4}, MoveOnly(), copy_only);
  auto& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceConstructionIlistWithCV) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type_t<const volatile ListMoveOnlyCopyOnly>(),
              {1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceConstructionIlistWithCVVariableTemplate) {
  const CopyOnly copy_only{};
  polly::any o(polly::in_place_type<const volatile ListMoveOnlyCopyOnly>, {1, 2, 3, 4}, MoveOnly(), copy_only);
  auto& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, InPlaceNoArgs) {
  polly::any o(polly::in_place_type_t<int>{});
  EXPECT_EQ(0, polly::any_cast<int&>(o));
}

TEST(AnyTest, InPlaceNoArgsVariableTemplate) {
  polly::any o(polly::in_place_type<int>);
  EXPECT_EQ(0, polly::any_cast<int&>(o));
}

template <typename Enabler, typename T, typename... Args>
struct CanEmplaceAnyImpl : std::false_type {};

template <typename T, typename... Args>
struct CanEmplaceAnyImpl<
    polly::void_t<decltype(
        std::declval<polly::any&>().emplace<T>(std::declval<Args>()...))>,
    T, Args...> : std::true_type {};

template <typename T, typename... Args>
using CanEmplaceAny = CanEmplaceAnyImpl<void, T, Args...>;

TEST(AnyTest, Emplace) {
  const CopyOnly copy_only{};
  polly::any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<IntMoveOnlyCopyOnly>(
                                5, MoveOnly(), copy_only)),
                            IntMoveOnlyCopyOnly&>::value));
  IntMoveOnlyCopyOnly& emplace_result =
      o.emplace<IntMoveOnlyCopyOnly>(5, MoveOnly(), copy_only);
  EXPECT_EQ(5, emplace_result.value);
  IntMoveOnlyCopyOnly& v = polly::any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
  EXPECT_EQ(&emplace_result, &v);

  static_assert(!CanEmplaceAny<int, int, int>::value, "");
  static_assert(!CanEmplaceAny<MoveOnly, MoveOnly>::value, "");
}

TEST(AnyTest, EmplaceWithCV) {
  const CopyOnly copy_only{};
  polly::any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<const volatile IntMoveOnlyCopyOnly>(
      5, MoveOnly(), copy_only)), IntMoveOnlyCopyOnly&>::value));
  IntMoveOnlyCopyOnly& emplace_result =
      o.emplace<const volatile IntMoveOnlyCopyOnly>(5, MoveOnly(), copy_only);
  EXPECT_EQ(5, emplace_result.value);
  IntMoveOnlyCopyOnly& v = polly::any_cast<IntMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(5, v.value);
  EXPECT_EQ(&emplace_result, &v);
}

TEST(AnyTest, EmplaceWithFunction) {
  polly::any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<FunctionType>(FunctionToEmplace)),
      FunctionType*&>::value));
  FunctionType*& emplace_result = o.emplace<FunctionType>(FunctionToEmplace);
  EXPECT_EQ(&FunctionToEmplace, emplace_result);
}

TEST(AnyTest, EmplaceWithArray) {
  polly::any o;
  ArrayType ar = {5, 42};
  EXPECT_TRUE((std::is_same<decltype(o.emplace<ArrayType>(ar)), DecayedArray&>::value));
  DecayedArray& emplace_result = o.emplace<ArrayType>(ar);
  EXPECT_EQ(&ar[0], emplace_result);
}

TEST(AnyTest, EmplaceIlist) {
  const CopyOnly copy_only{};
  polly::any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<ListMoveOnlyCopyOnly>(
      {1, 2, 3, 4}, MoveOnly(), copy_only)), ListMoveOnlyCopyOnly&>::value));
  ListMoveOnlyCopyOnly& emplace_result =
      o.emplace<ListMoveOnlyCopyOnly>({1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(&v, &emplace_result);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);

  static_assert(!CanEmplaceAny<int, std::initializer_list<int>>::value, "");
  static_assert(!CanEmplaceAny<MoveOnlyWithListConstructor,
                               std::initializer_list<int>, int>::value, "");
}

TEST(AnyTest, EmplaceIlistWithCV) {
  const CopyOnly copy_only{};
  polly::any o;
  EXPECT_TRUE((std::is_same<decltype(o.emplace<const volatile ListMoveOnlyCopyOnly>(
      {1, 2, 3, 4}, MoveOnly(), copy_only)), ListMoveOnlyCopyOnly&>::value));
  ListMoveOnlyCopyOnly& emplace_result =
      o.emplace<const volatile ListMoveOnlyCopyOnly>({1, 2, 3, 4}, MoveOnly(), copy_only);
  ListMoveOnlyCopyOnly& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  EXPECT_EQ(&v, &emplace_result);
  std::vector<int> expected_values = {1, 2, 3, 4};
  EXPECT_EQ(expected_values, v.values);
}

TEST(AnyTest, EmplaceNoArgs) {
  polly::any o;
  o.emplace<int>();
  EXPECT_EQ(0, polly::any_cast<int>(o));
}

TEST(AnyTest, Swap) {
  polly::any f0(100);
  polly::any f1(200);

  EXPECT_EQ(polly::any_cast<int>(f0), 100);
  EXPECT_EQ(polly::any_cast<int>(f1), 200);
  using std::swap;
  swap(f0, f1);
  EXPECT_EQ(polly::any_cast<int>(f0), 200);
  EXPECT_EQ(polly::any_cast<int>(f1), 100);
}

TEST(AnyTest, ConversionConstruction) {
  {
    polly::any o = 5;
    EXPECT_EQ(5, polly::any_cast<int>(o));
  }

  {
    const CopyOnly copy_only(5);
    polly::any o = copy_only;
    EXPECT_EQ(5, polly::any_cast<CopyOnly&>(o).value);
  }

  static_assert(!std::is_convertible<MoveOnly, polly::any>::value, "");
}

TEST(AnyTest, ConversionAssignment) {
  {
    polly::any o;
    o = 5;
    EXPECT_EQ(5, polly::any_cast<int>(o));
  }

  {
    const CopyOnly copy_only(5);
    polly::any o;
    o = copy_only;
    EXPECT_EQ(5, polly::any_cast<CopyOnly&>(o).value);
  }

  static_assert(!std::is_assignable<MoveOnly, polly::any>::value, "");
}

// Weird type for testing, only used to make sure we "properly" perfect-forward
// when being placed into an polly::any (use the l-value constructor if given an
// l-value rather than use the copy constructor).
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
    polly::any o = source;  // Actual copy
    EXPECT_EQ(5, polly::any_cast<WeirdConstructor42&>(o).value);
  }

  {
    WeirdConstructor42 source(5);
    polly::any o = source;  // Weird "conversion"
    EXPECT_EQ(42, polly::any_cast<WeirdConstructor42&>(o).value);
  }
}

TEST(AnyTest, WeirdConversionAssignment) {
  {
    const WeirdConstructor42 source(5);
    polly::any o;
    o = source;  // Actual copy
    EXPECT_EQ(5, polly::any_cast<WeirdConstructor42&>(o).value);
  }

  {
    WeirdConstructor42 source(5);
    polly::any o;
    o = source;  // Weird "conversion"
    EXPECT_EQ(42, polly::any_cast<WeirdConstructor42&>(o).value);
  }
}

struct Value {};

TEST(AnyTest, AnyCastValue) {
  {
    polly::any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, polly::any_cast<int>(o));
    EXPECT_EQ(5, polly::any_cast<int>(AsConst(o)));
    static_assert(std::is_same<decltype(polly::any_cast<Value>(o)), Value>::value, "");
  }

  {
    polly::any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, polly::any_cast<const int>(o));
    EXPECT_EQ(5, polly::any_cast<const int>(AsConst(o)));
    static_assert(std::is_same<decltype(polly::any_cast<const Value>(o)), const Value>::value,  "");
  }
}

TEST(AnyTest, AnyCastReference) {
  {
    polly::any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, polly::any_cast<int&>(o));
    EXPECT_EQ(5, polly::any_cast<const int&>(AsConst(o)));
    static_assert(std::is_same<decltype(polly::any_cast<Value&>(o)), Value&>::value, "");
  }

  {
    polly::any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, polly::any_cast<const int>(o));
    EXPECT_EQ(5, polly::any_cast<const int>(AsConst(o)));
    static_assert(std::is_same<decltype(polly::any_cast<const Value&>(o)), const Value&>::value, "");
  }

  {
    polly::any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, polly::any_cast<int&&>(std::move(o)));
    static_assert(std::is_same<decltype(polly::any_cast<Value&&>(std::move(o))), Value&&>::value, "");
  }

  {
    polly::any o;
    o.emplace<int>(5);
    EXPECT_EQ(5, polly::any_cast<const int>(std::move(o)));
    static_assert(std::is_same<decltype(polly::any_cast<const Value&&>(std::move(o))), const Value&&>::value, "");
  }
}

TEST(AnyTest, AnyCastPointer) {
  {
    polly::any o;
    EXPECT_EQ(nullptr, polly::any_cast<char>(&o));
    o.emplace<int>(5);
    EXPECT_EQ(nullptr, polly::any_cast<char>(&o));
    o.emplace<char>('a');
    EXPECT_EQ('a', *polly::any_cast<char>(&o));
    static_assert(std::is_same<decltype(polly::any_cast<Value>(&o)), Value*>::value, "");
  }

  {
    polly::any o;
    EXPECT_EQ(nullptr, polly::any_cast<const char>(&o));
    o.emplace<int>(5);
    EXPECT_EQ(nullptr, polly::any_cast<const char>(&o));
    o.emplace<char>('a');
    EXPECT_EQ('a', *polly::any_cast<const char>(&o));
    static_assert(std::is_same<decltype(polly::any_cast<const Value>(&o)), const Value*>::value, "");
  }
}

TEST(AnyTest, MakeAny) {
  const CopyOnly copy_only{};
  auto o = polly::make_any<IntMoveOnlyCopyOnly>(5, MoveOnly(), copy_only);
  static_assert(std::is_same<decltype(o), polly::any>::value, "");
  EXPECT_EQ(5, polly::any_cast<IntMoveOnlyCopyOnly&>(o).value);
}

TEST(AnyTest, MakeAnyIList) {
  const CopyOnly copy_only{};
  auto o =
      polly::make_any<ListMoveOnlyCopyOnly>({1, 2, 3}, MoveOnly(), copy_only);
  static_assert(std::is_same<decltype(o), polly::any>::value, "");
  ListMoveOnlyCopyOnly& v = polly::any_cast<ListMoveOnlyCopyOnly&>(o);
  std::vector<int> expected_values = {1, 2, 3};
  EXPECT_EQ(expected_values, v.values);
}

// Test the use of copy constructor and operator=
TEST(AnyTest, Copy) {
  {
    polly::any o(polly::in_place_type<int>, 123);
    int* f1 = polly::any_cast<int>(&o);

    polly::any o2(o);
    const int* f2 = polly::any_cast<int>(&o2);
    EXPECT_EQ(123, *f2);
    EXPECT_NE(f1, f2);

    polly::any o3;
    o3 = o2;
    const int* f3 = polly::any_cast<int>(&o3);
    EXPECT_EQ(123, *f3);
    EXPECT_NE(f2, f3);

    const polly::any o4(4);
    // copy construct from const lvalue ref.
    polly::any o5 = o4;
    EXPECT_EQ(4, polly::any_cast<int>(o4));
    EXPECT_EQ(4, polly::any_cast<int>(o5));

    // Copy construct from const rvalue ref.
    polly::any o6 = std::move(o4);  // NOLINT
    EXPECT_EQ(4, polly::any_cast<int>(o4));
    EXPECT_EQ(4, polly::any_cast<int>(o6));
  }
}

TEST(AnyTest, Move) {
  polly::any tmp4(4);
  polly::any o4(std::move(tmp4));  // move construct
  EXPECT_EQ(4, polly::any_cast<int>(o4));
  o4 = *&o4;  // self assign
  EXPECT_EQ(4, polly::any_cast<int>(o4));
  EXPECT_TRUE(o4.has_value());

  polly::any o5;
  polly::any tmp5(5);
  o5 = std::move(tmp5);  // move assign
  EXPECT_EQ(5, polly::any_cast<int>(o5));
}

// Reset the ObjectOwner with an object of a different type
TEST(AnyTest, Reset) {
  polly::any o;
  o.emplace<int>();

  o.reset();
  EXPECT_FALSE(o.has_value());

  o.emplace<char>();
  EXPECT_TRUE(o.has_value());
}

#ifdef POLLY_HAVE_EXCEPTIONS
// If using the absl `any` implementation, we can rely on a specific message.
#define POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(expr) do {             \
  bool flags = false;                                             \
  try { expr; } catch (polly::bad_any_cast& e) { flags = true; }  \
  EXPECT_TRUE(flags);                                             \
} while (0)

TEST(AnyTest, ThrowBadAlloc) {
  {
    polly::any a;
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<int&>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const int&>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<int&&>(polly::any{}));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const int&&>(polly::any{}));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<int>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const int>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<int>(polly::any{}));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const int>(polly::any{}));

    // const polly::any operand
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const int&>(AsConst(a)));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<int>(AsConst(a)));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const int>(AsConst(a)));
  }

  {
    polly::any a(polly::in_place_type<int>);
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<float&>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const float&>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<float&&>(polly::any{}));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const float&&>(polly::any{}));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<float>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const float>(a));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<float>(polly::any{}));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const float>(polly::any{}));

    // const polly::any operand
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const float&>(AsConst(a)));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<float>(AsConst(a)));
    POLLY_ANY_TEST_EXPECT_BAD_ANY_CAST(polly::any_cast<const float>(AsConst(a)));
  }
}
#endif // POLLY_HAVE_EXCEPTIONS

}  // namespace

#endif  // POLLY_HAVE_STD_OPTIONAL
