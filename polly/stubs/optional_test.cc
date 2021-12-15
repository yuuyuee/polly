// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Modify
//  1. adapted to polly
//  2. commeneted for the initialize-list initialized functions.

#include "stubs/config.h"
#include <iostream>
// This test is a no-op when polly::optional is an alias for std::optional.

#ifndef POLLY_HAVE_STD_OPTIONAL

#include "stubs/optional.h"

#include <string>
#include <type_traits>
#include <utility>

#include "gtest/gtest.h"
#include "stubs/type_traits.h"
#include "stubs/string_view.h"

struct Hashable {};

namespace std {
template <>
struct hash<Hashable> {
  size_t operator()(const Hashable&) { return 0; }
};
}  // namespace std

struct NonHashable {};

namespace {

std::string TypeQuals(std::string&) { return "&"; }
std::string TypeQuals(std::string&&) { return "&&"; }
std::string TypeQuals(const std::string&) { return "c&"; }
std::string TypeQuals(const std::string&&) { return "c&&"; }

struct StructorListener {
  int construct0 = 0;
  int construct1 = 0;
  int construct2 = 0;
  int listinit = 0;
  int copy = 0;
  int move = 0;
  int copy_assign = 0;
  int move_assign = 0;
  int destruct = 0;
  int volatile_copy = 0;
  int volatile_move = 0;
  int volatile_copy_assign = 0;
  int volatile_move_assign = 0;
};

// Suppress MSVC warnings.
// 4521: multiple copy constructors specified
// 4522: multiple assignment operators specified
// We wrote multiple of them to test that the correct overloads are selected.
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4521)
#pragma warning( disable : 4522)
#endif
struct Listenable {
  static StructorListener* listener;

  Listenable() { ++listener->construct0; }
  explicit Listenable(int /*unused*/) { ++listener->construct1; }
  Listenable(int /*unused*/, int /*unused*/) { ++listener->construct2; }
  Listenable(std::initializer_list<int> /*unused*/) { ++listener->listinit; }
  Listenable(const Listenable& /*unused*/) { ++listener->copy; }
  Listenable(const volatile Listenable& /*unused*/) {
    ++listener->volatile_copy;
  }
  Listenable(volatile Listenable&& /*unused*/) { ++listener->volatile_move; }
  Listenable(Listenable&& /*unused*/) { ++listener->move; }
  Listenable& operator=(const Listenable& /*unused*/) {
    ++listener->copy_assign;
    return *this;
  }
  Listenable& operator=(Listenable&& /*unused*/) {
    ++listener->move_assign;
    return *this;
  }
  // use void return type instead of volatile T& to work around GCC warning
  // when the assignment's returned reference is ignored.
  void operator=(const volatile Listenable& /*unused*/) volatile {
    ++listener->volatile_copy_assign;
  }
  void operator=(volatile Listenable&& /*unused*/) volatile {
    ++listener->volatile_move_assign;
  }
  ~Listenable() { ++listener->destruct; }
};
#ifdef _MSC_VER
#pragma warning( pop )
#endif

StructorListener* Listenable::listener = nullptr;

// POLLY_HAVE_NO_CONSTEXPR_INITIALIZER_LIST is defined to 1 when the standard
// library implementation doesn't marked initializer_list's default constructor
// constexpr. The C++11 standard doesn't specify constexpr on it, but C++14
// added it. However, libstdc++ 4.7 marked it constexpr.
#if defined(_LIBCPP_VERSION) && \
    (_LIBCPP_STD_VER <= 11 || defined(_LIBCPP_HAS_NO_CXX14_CONSTEXPR))
#define POLLY_HAVE_NO_CONSTEXPR_INITIALIZER_LIST 1
#endif

struct ConstexprType {
  enum CtorTypes {
    kCtorDefault,
    kCtorInt,
    kCtorInitializerList,
    kCtorConstChar
  };
  constexpr ConstexprType() : x(kCtorDefault) {}
  constexpr explicit ConstexprType(int) : x(kCtorInt) {}
#ifndef POLLY_HAVE_NO_CONSTEXPR_INITIALIZER_LIST
  constexpr ConstexprType(std::initializer_list<int>)
      : x(kCtorInitializerList) {}
#endif
  constexpr ConstexprType(const char*)  // NOLINT(runtime/explicit)
      : x(kCtorConstChar) {}
  int x;
};

struct Copyable {
  Copyable() {}
  Copyable(const Copyable&) {}
  Copyable& operator=(const Copyable&) { return *this; }
};

struct MoveableThrow {
  MoveableThrow() {}
  MoveableThrow(MoveableThrow&&) {}
  MoveableThrow& operator=(MoveableThrow&&) { return *this; }
};

struct MoveableNoThrow {
  MoveableNoThrow() {}
  MoveableNoThrow(MoveableNoThrow&&) noexcept {}
  MoveableNoThrow& operator=(MoveableNoThrow&&) noexcept { return *this; }
};

struct NonMovable {
  NonMovable() {}
  NonMovable(const NonMovable&) = delete;
  NonMovable& operator=(const NonMovable&) = delete;
  NonMovable(NonMovable&&) = delete;
  NonMovable& operator=(NonMovable&&) = delete;
};

struct NoDefault {
  NoDefault() = delete;
  NoDefault(const NoDefault&) {}
  NoDefault& operator=(const NoDefault&) { return *this; }
};

struct ConvertsFromInPlaceT {
  ConvertsFromInPlaceT(polly::in_place_t) {}  // NOLINT
};

TEST(optionalTest, DefaultConstructor) {
  polly::optional<int> empty;
  EXPECT_FALSE(empty);
  constexpr polly::optional<int> cempty;
  static_assert(!cempty.has_value(), "");
  EXPECT_TRUE(std::is_nothrow_default_constructible<polly::optional<int>>::value);
}

TEST(optionalTest, nulloptConstructor) {
  polly::optional<int> empty(polly::nullopt);
  EXPECT_FALSE(empty);
  constexpr polly::optional<int> cempty{polly::nullopt};
  static_assert(!cempty.has_value(), "");
  EXPECT_TRUE((std::is_nothrow_constructible<polly::optional<int>,
                                             polly::nullopt_t>::value));
}

TEST(optionalTest, CopyConstructor) {
  {
    polly::optional<int> empty, opt42 = 42;
    polly::optional<int> empty_copy(empty);
    EXPECT_FALSE(empty_copy);
    polly::optional<int> opt42_copy(opt42);
    EXPECT_TRUE(opt42_copy);
    EXPECT_EQ(42, *opt42_copy);
  }
  {
    polly::optional<const int> empty, opt42 = 42;
    polly::optional<const int> empty_copy(empty);
    EXPECT_FALSE(empty_copy);
    polly::optional<const int> opt42_copy(opt42);
    EXPECT_TRUE(opt42_copy);
    EXPECT_EQ(42, *opt42_copy);
  }
  {
    polly::optional<volatile int> empty, opt42 = 42;
    polly::optional<volatile int> empty_copy(empty);
    EXPECT_FALSE(empty_copy);
    polly::optional<volatile int> opt42_copy(opt42);
    EXPECT_TRUE(opt42_copy);
    EXPECT_EQ(42, *opt42_copy);
  }
  // test copyablility
  EXPECT_TRUE(std::is_copy_constructible<polly::optional<int>>::value);
  EXPECT_TRUE(std::is_copy_constructible<polly::optional<Copyable>>::value);
  EXPECT_FALSE(std::is_copy_constructible<polly::optional<MoveableThrow>>::value);
  EXPECT_FALSE(std::is_copy_constructible<polly::optional<MoveableNoThrow>>::value);
  EXPECT_FALSE(std::is_copy_constructible<polly::optional<NonMovable>>::value);

  EXPECT_FALSE(polly::is_trivially_copy_constructible<polly::optional<Copyable>>::value);

#if defined(POLLY_HAVE_STD_OPTIONAL) && defined(__GLIBCXX__)
  // libstdc++ std::optional implementation (as of 7.2) has a bug: when T is
  // trivially copyable, optional<T> is not trivially copyable (due to one of
  // its base class is unconditionally nontrivial).
#define POLLY_GLIBCXX_OPTIONAL_TRIVIALITY_BUG 1
#endif

#ifndef POLLY_GLIBCXX_OPTIONAL_TRIVIALITY_BUG
  EXPECT_TRUE(polly::is_trivially_copy_constructible<polly::optional<int>>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<polly::optional<const int>>::value);
#ifndef _MSC_VER
  // See defect report "Trivial copy/move constructor for class with volatile
  // member" at
  // http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#2094
  // A class with non-static data member of volatile-qualified type should still
  // have a trivial copy constructor if the data member is trivial.
  // Also a cv-qualified scalar type should be trivially copyable.
  EXPECT_TRUE(polly::is_trivially_copy_constructible<
              polly::optional<volatile int>>::value);
#endif  // _MSC_VER
#endif  // POLLY_GLIBCXX_OPTIONAL_TRIVIALITY_BUG

  // constexpr copy constructor for trivially copyable types
  {
    constexpr polly::optional<int> o1;
    constexpr polly::optional<int> o2 = o1;
    static_assert(!o2, "");
  }
  {
    constexpr polly::optional<int> o1 = 42;
    constexpr polly::optional<int> o2 = o1;
    static_assert(o2, "");
    static_assert(*o2 == 42, "");
  }
  {
    struct TrivialCopyable {
      constexpr TrivialCopyable() : x(0) {}
      constexpr explicit TrivialCopyable(int i) : x(i) {}
      int x;
    };
    constexpr polly::optional<TrivialCopyable> o1(42);
    constexpr polly::optional<TrivialCopyable> o2 = o1;
    static_assert(o2, "");
    static_assert((*o2).x == 42, "");
#ifndef POLLY_GLIBCXX_OPTIONAL_TRIVIALITY_BUG
    EXPECT_TRUE(polly::is_trivially_copy_constructible<
                polly::optional<TrivialCopyable>>::value);
    EXPECT_TRUE(polly::is_trivially_copy_constructible<
                polly::optional<const TrivialCopyable>>::value);
#endif
    // When testing with VS 2017 15.3, there seems to be a bug in MSVC
    // std::optional when T is volatile-qualified. So skipping this test.
    // Bug report:
    // https://connect.microsoft.com/VisualStudio/feedback/details/3142534
#if defined(POLLY_USES_STD_OPTIONAL) && defined(_MSC_VER) && _MSC_VER >= 1911
#define POLLY_MSVC_OPTIONAL_VOLATILE_COPY_BUG 1
#endif
#ifndef POLLY_MSVC_OPTIONAL_VOLATILE_COPY_BUG
    EXPECT_FALSE(std::is_copy_constructible<
                 polly::optional<volatile TrivialCopyable>>::value);
#endif
  }
}

TEST(optionalTest, MoveConstructor) {
  polly::optional<int> empty, opt42 = 42;
  polly::optional<int> empty_move(std::move(empty));
  EXPECT_FALSE(empty_move);
  polly::optional<int> opt42_move(std::move(opt42));
  EXPECT_TRUE(opt42_move);
  EXPECT_EQ(42, opt42_move);
  // test movability
  EXPECT_TRUE(std::is_move_constructible<polly::optional<int>>::value);
  EXPECT_TRUE(std::is_move_constructible<polly::optional<Copyable>>::value);
  EXPECT_TRUE(std::is_move_constructible<polly::optional<MoveableThrow>>::value);
  EXPECT_TRUE(std::is_move_constructible<polly::optional<MoveableNoThrow>>::value);
  EXPECT_FALSE(std::is_move_constructible<polly::optional<NonMovable>>::value);
  // test noexcept
  EXPECT_TRUE(std::is_nothrow_move_constructible<polly::optional<int>>::value);
#ifndef POLLY_USES_STD_OPTIONAL
  // EXPECT_EQ(
  //     std::default_allocator_is_nothrow::value,
  //     std::is_nothrow_move_constructible<polly::optional<MoveableThrow>>::value);
#endif
  EXPECT_TRUE(std::is_nothrow_move_constructible<
              polly::optional<MoveableNoThrow>>::value);
}

TEST(optionalTest, Destructor) {
  struct Trivial {};

  struct NonTrivial {
    NonTrivial(const NonTrivial&) {}
    NonTrivial& operator=(const NonTrivial&) { return *this; }
    ~NonTrivial() {}
  };

  EXPECT_TRUE(std::is_trivially_destructible<polly::optional<int>>::value);
  EXPECT_TRUE(std::is_trivially_destructible<polly::optional<Trivial>>::value);
  EXPECT_FALSE(
      std::is_trivially_destructible<polly::optional<NonTrivial>>::value);
}

TEST(optionalTest, InPlaceConstructor) {
  constexpr polly::optional<ConstexprType> opt0{polly::in_place_t()};
  static_assert(opt0, "");
  static_assert((*opt0).x == ConstexprType::kCtorDefault, "");
  constexpr polly::optional<ConstexprType> opt1{polly::in_place_t(), 1};
  static_assert(opt1, "");
  static_assert((*opt1).x == ConstexprType::kCtorInt, "");
#ifndef POLLY_HAVE_NO_CONSTEXPR_INITIALIZER_LIST
  constexpr polly::optional<ConstexprType> opt2{polly::in_place_t(), {1, 2}};
  static_assert(opt2, "");
  static_assert((*opt2).x == ConstexprType::kCtorInitializerList, "");
#endif
  EXPECT_FALSE((std::is_constructible<polly::optional<ConvertsFromInPlaceT>,
                                      polly::in_place_t>::value));
  EXPECT_FALSE((std::is_constructible<polly::optional<ConvertsFromInPlaceT>,
                                      const polly::in_place_t&>::value));
  EXPECT_TRUE(
      (std::is_constructible<polly::optional<ConvertsFromInPlaceT>,
                             polly::in_place_t, polly::in_place_t>::value));

  EXPECT_FALSE((std::is_constructible<polly::optional<NoDefault>,
                                      polly::in_place_t>::value));
  EXPECT_FALSE((std::is_constructible<polly::optional<NoDefault>,
                                      polly::in_place_t&&>::value));
}

// template<U=T> optional(U&&);
TEST(optionalTest, ValueConstructor) {
  constexpr polly::optional<int> opt0(0);
  static_assert(opt0, "");
  static_assert(*opt0 == 0, "");
  EXPECT_TRUE((std::is_convertible<int, polly::optional<int>>::value));
  // Copy initialization ( = "abc") won't work due to optional(optional&&)
  // is not constexpr. Use list initialization instead. This invokes
  // polly::optional<ConstexprType>::polly::optional<U>(U&&), with U = const char
  // (&) [4], which direct-initializes the ConstexprType value held by the
  // optional via ConstexprType::ConstexprType(const char*).
  constexpr polly::optional<ConstexprType> opt1 = {"abc"};
  static_assert(opt1, "");
  static_assert(ConstexprType::kCtorConstChar == (*opt1).x, "");
  EXPECT_TRUE(
      (std::is_convertible<const char*, polly::optional<ConstexprType>>::value));
  // direct initialization
  constexpr polly::optional<ConstexprType> opt2{2};
  static_assert(opt2, "");
  static_assert(ConstexprType::kCtorInt == (*opt2).x, "");
  EXPECT_FALSE(
      (std::is_convertible<int, polly::optional<ConstexprType>>::value));

  // this invokes polly::optional<int>::optional(int&&)
  // NOTE: this has different behavior than assignment, e.g.
  // "opt3 = {};" clears the optional rather than setting the value to 0
  // According to C++17 standard N4659 [over.ics.list] 16.3.3.1.5, (9.2)- "if
  // the initializer list has no elements, the implicit conversion is the
  // identity conversion", so `optional(int&&)` should be a better match than
  // `optional(optional&&)` which is a user-defined conversion.
  // Note: GCC 7 has a bug with this overload selection when compiled with
  // `-std=c++17`.
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ == 7 && \
    __cplusplus == 201703L
#define POLLY_GCC7_OVER_ICS_LIST_BUG 1
#endif
#ifndef POLLY_GCC7_OVER_ICS_LIST_BUG
  constexpr polly::optional<int> opt3({});
  static_assert(opt3, "");
  static_assert(*opt3 == 0, "");
#endif

  // this invokes the move constructor with a default constructed optional
  // because non-template function is a better match than template function.
  polly::optional<ConstexprType> opt4({});
  EXPECT_FALSE(opt4);
}

struct Implicit {};

struct Explicit {};

struct Convert {
  Convert(const Implicit&)  // NOLINT(runtime/explicit)
      : implicit(true), move(false) {}
  Convert(Implicit&&)  // NOLINT(runtime/explicit)
      : implicit(true), move(true) {}
  explicit Convert(const Explicit&) : implicit(false), move(false) {}
  explicit Convert(Explicit&&) : implicit(false), move(true) {}

  bool implicit;
  bool move;
};

struct ConvertFromOptional {
  ConvertFromOptional(const Implicit&)  // NOLINT(runtime/explicit)
      : implicit(true), move(false), from_optional(false) {}
  ConvertFromOptional(Implicit&&)  // NOLINT(runtime/explicit)
      : implicit(true), move(true), from_optional(false) {}
  ConvertFromOptional(
      const polly::optional<Implicit>&)  // NOLINT(runtime/explicit)
      : implicit(true), move(false), from_optional(true) {}
  ConvertFromOptional(polly::optional<Implicit>&&)  // NOLINT(runtime/explicit)
      : implicit(true), move(true), from_optional(true) {}
  explicit ConvertFromOptional(const Explicit&)
      : implicit(false), move(false), from_optional(false) {}
  explicit ConvertFromOptional(Explicit&&)
      : implicit(false), move(true), from_optional(false) {}
  explicit ConvertFromOptional(const polly::optional<Explicit>&)
      : implicit(false), move(false), from_optional(true) {}
  explicit ConvertFromOptional(polly::optional<Explicit>&&)
      : implicit(false), move(true), from_optional(true) {}

  bool implicit;
  bool move;
  bool from_optional;
};

TEST(optionalTest, ConvertingConstructor) {
  polly::optional<Implicit> i_empty;
  polly::optional<Implicit> i(polly::in_place);
  polly::optional<Explicit> e_empty;
  polly::optional<Explicit> e(polly::in_place);
  {
    // implicitly constructing polly::optional<Convert> from
    // polly::optional<Implicit>
    polly::optional<Convert> empty = i_empty;
    EXPECT_FALSE(empty);
    polly::optional<Convert> opt_copy = i;
    EXPECT_TRUE(opt_copy);
    EXPECT_TRUE(opt_copy->implicit);
    EXPECT_FALSE(opt_copy->move);
    polly::optional<Convert> opt_move = polly::optional<Implicit>(polly::in_place);
    EXPECT_TRUE(opt_move);
    EXPECT_TRUE(opt_move->implicit);
    EXPECT_TRUE(opt_move->move);
  }
  {
    // explicitly constructing polly::optional<Convert> from
    // polly::optional<Explicit>
    polly::optional<Convert> empty(e_empty);
    EXPECT_FALSE(empty);
    polly::optional<Convert> opt_copy(e);
    EXPECT_TRUE(opt_copy);
    EXPECT_FALSE(opt_copy->implicit);
    EXPECT_FALSE(opt_copy->move);
    EXPECT_FALSE((std::is_convertible<const polly::optional<Explicit>&,
                                      polly::optional<Convert>>::value));
    polly::optional<Convert> opt_move{polly::optional<Explicit>(polly::in_place)};
    EXPECT_TRUE(opt_move);
    EXPECT_FALSE(opt_move->implicit);
    EXPECT_TRUE(opt_move->move);
    EXPECT_FALSE((std::is_convertible<polly::optional<Explicit>&&,
                                      polly::optional<Convert>>::value));
  }
  {
    // implicitly constructing polly::optional<ConvertFromOptional> from
    // polly::optional<Implicit> via
    // ConvertFromOptional(polly::optional<Implicit>&&) check that
    // ConvertFromOptional(Implicit&&) is NOT called
    static_assert(
        std::is_convertible<polly::optional<Implicit>,
                            polly::optional<ConvertFromOptional>>::value,
        "");
    polly::optional<ConvertFromOptional> opt0 = i_empty;
    EXPECT_TRUE(opt0);
    EXPECT_TRUE(opt0->implicit);
    EXPECT_FALSE(opt0->move);
    EXPECT_TRUE(opt0->from_optional);
    polly::optional<ConvertFromOptional> opt1 = polly::optional<Implicit>();
    EXPECT_TRUE(opt1);
    EXPECT_TRUE(opt1->implicit);
    EXPECT_TRUE(opt1->move);
    EXPECT_TRUE(opt1->from_optional);
  }
  {
    // implicitly constructing polly::optional<ConvertFromOptional> from
    // polly::optional<Explicit> via
    // ConvertFromOptional(polly::optional<Explicit>&&) check that
    // ConvertFromOptional(Explicit&&) is NOT called
    polly::optional<ConvertFromOptional> opt0(e_empty);
    EXPECT_TRUE(opt0);
    EXPECT_FALSE(opt0->implicit);
    EXPECT_FALSE(opt0->move);
    EXPECT_TRUE(opt0->from_optional);
    EXPECT_FALSE(
        (std::is_convertible<const polly::optional<Explicit>&,
                             polly::optional<ConvertFromOptional>>::value));
    polly::optional<ConvertFromOptional> opt1{polly::optional<Explicit>()};
    EXPECT_TRUE(opt1);
    EXPECT_FALSE(opt1->implicit);
    EXPECT_TRUE(opt1->move);
    EXPECT_TRUE(opt1->from_optional);
    EXPECT_FALSE(
        (std::is_convertible<polly::optional<Explicit>&&,
                             polly::optional<ConvertFromOptional>>::value));
  }
}

TEST(optionalTest, StructorBasic) {
  StructorListener listener;
  Listenable::listener = &listener;
  {
    polly::optional<Listenable> empty;
    EXPECT_FALSE(empty);
    polly::optional<Listenable> opt0(polly::in_place);
    EXPECT_TRUE(opt0);
    polly::optional<Listenable> opt1(polly::in_place, 1);
    EXPECT_TRUE(opt1);
    polly::optional<Listenable> opt2(polly::in_place, 1, 2);
    EXPECT_TRUE(opt2);
  }
  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(1, listener.construct1);
  EXPECT_EQ(1, listener.construct2);
  EXPECT_EQ(3, listener.destruct);
}

TEST(optionalTest, CopyMoveStructor) {
  StructorListener listener;
  Listenable::listener = &listener;
  polly::optional<Listenable> original(polly::in_place);
  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(0, listener.copy);
  EXPECT_EQ(0, listener.move);
  polly::optional<Listenable> copy(original);
  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(1, listener.copy);
  EXPECT_EQ(0, listener.move);
  polly::optional<Listenable> move(std::move(original));
  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(1, listener.copy);
  EXPECT_EQ(1, listener.move);
}

TEST(optionalTest, ListInit) {
  StructorListener listener;
  Listenable::listener = &listener;
  polly::optional<Listenable> listinit1(polly::in_place, {1});
  polly::optional<Listenable> listinit2(polly::in_place, {1, 2});
  EXPECT_EQ(2, listener.listinit);
}

TEST(optionalTest, AssignFromNullopt) {
  polly::optional<int> opt(1);
  opt = polly::nullopt;
  EXPECT_FALSE(opt);

  StructorListener listener;
  Listenable::listener = &listener;
  polly::optional<Listenable> opt1(polly::in_place);
  opt1 = polly::nullopt;
  EXPECT_FALSE(opt1);
  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(1, listener.destruct);

  EXPECT_TRUE((
      std::is_nothrow_assignable<polly::optional<int>, polly::nullopt_t>::value));
  EXPECT_TRUE((std::is_nothrow_assignable<polly::optional<Listenable>,
                                          polly::nullopt_t>::value));
}

TEST(optionalTest, CopyAssignment) {
  const polly::optional<int> empty, opt1 = 1, opt2 = 2;
  polly::optional<int> empty_to_opt1, opt1_to_opt2, opt2_to_empty;

  EXPECT_FALSE(empty_to_opt1);
  empty_to_opt1 = empty;
  EXPECT_FALSE(empty_to_opt1);
  empty_to_opt1 = opt1;
  EXPECT_TRUE(empty_to_opt1);
  EXPECT_EQ(1, empty_to_opt1.value());

  EXPECT_FALSE(opt1_to_opt2);
  opt1_to_opt2 = opt1;
  EXPECT_TRUE(opt1_to_opt2);
  EXPECT_EQ(1, opt1_to_opt2.value());
  opt1_to_opt2 = opt2;
  EXPECT_TRUE(opt1_to_opt2);
  EXPECT_EQ(2, opt1_to_opt2.value());

  EXPECT_FALSE(opt2_to_empty);
  opt2_to_empty = opt2;
  EXPECT_TRUE(opt2_to_empty);
  EXPECT_EQ(2, opt2_to_empty.value());
  opt2_to_empty = empty;
  EXPECT_FALSE(opt2_to_empty);

  EXPECT_FALSE(std::is_copy_assignable<polly::optional<const int>>::value);
  EXPECT_TRUE(std::is_copy_assignable<polly::optional<Copyable>>::value);
  EXPECT_FALSE(std::is_copy_assignable<polly::optional<MoveableThrow>>::value);
  EXPECT_FALSE(
      std::is_copy_assignable<polly::optional<MoveableNoThrow>>::value);
  EXPECT_FALSE(std::is_copy_assignable<polly::optional<NonMovable>>::value);

  EXPECT_TRUE(polly::is_trivially_copy_assignable<int>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<volatile int>::value);

  struct Trivial {
    int i;
  };
  struct NonTrivial {
    NonTrivial& operator=(const NonTrivial&) { return *this; }
    int i;
  };

  EXPECT_TRUE(polly::is_trivially_copy_assignable<Trivial>::value);
  EXPECT_FALSE(std::is_copy_assignable<const Trivial>::value);
  EXPECT_FALSE(std::is_copy_assignable<volatile Trivial>::value);
  EXPECT_TRUE(std::is_copy_assignable<NonTrivial>::value);
  EXPECT_FALSE(polly::is_trivially_copy_assignable<NonTrivial>::value);

  // std::optional doesn't support volatile nontrivial types.
#ifndef POLLY_USES_STD_OPTIONAL
  {
    StructorListener listener;
    Listenable::listener = &listener;

    polly::optional<volatile Listenable> empty, set(polly::in_place);
    EXPECT_EQ(1, listener.construct0);
    polly::optional<volatile Listenable> empty_to_empty, empty_to_set,
        set_to_empty(polly::in_place), set_to_set(polly::in_place);
    EXPECT_EQ(3, listener.construct0);
    empty_to_empty = empty;  // no effect
    empty_to_set = set;      // copy construct
    set_to_empty = empty;    // destruct
    set_to_set = set;        // copy assign
    EXPECT_EQ(1, listener.volatile_copy);
    EXPECT_EQ(0, listener.volatile_move);
    EXPECT_EQ(1, listener.destruct);
    EXPECT_EQ(1, listener.volatile_copy_assign);
  }
#endif  // POLLY_USES_STD_OPTIONAL
}

TEST(optionalTest, MoveAssignment) {
  {
    StructorListener listener;
    Listenable::listener = &listener;

    polly::optional<Listenable> empty1, empty2, set1(polly::in_place),
        set2(polly::in_place);
    EXPECT_EQ(2, listener.construct0);
    polly::optional<Listenable> empty_to_empty, empty_to_set,
        set_to_empty(polly::in_place), set_to_set(polly::in_place);
    EXPECT_EQ(4, listener.construct0);
    empty_to_empty = std::move(empty1);
    empty_to_set = std::move(set1);
    set_to_empty = std::move(empty2);
    set_to_set = std::move(set2);
    EXPECT_EQ(0, listener.copy);
    EXPECT_EQ(1, listener.move);
    EXPECT_EQ(1, listener.destruct);
    EXPECT_EQ(1, listener.move_assign);
  }
  // std::optional doesn't support volatile nontrivial types.
#ifndef POLLY_USES_STD_OPTIONAL
  {
    StructorListener listener;
    Listenable::listener = &listener;

    polly::optional<volatile Listenable> empty1, empty2, set1(polly::in_place),
        set2(polly::in_place);
    EXPECT_EQ(2, listener.construct0);
    polly::optional<volatile Listenable> empty_to_empty, empty_to_set,
        set_to_empty(polly::in_place), set_to_set(polly::in_place);
    EXPECT_EQ(4, listener.construct0);
    empty_to_empty = std::move(empty1);  // no effect
    empty_to_set = std::move(set1);      // move construct
    set_to_empty = std::move(empty2);    // destruct
    set_to_set = std::move(set2);        // move assign
    EXPECT_EQ(0, listener.volatile_copy);
    EXPECT_EQ(1, listener.volatile_move);
    EXPECT_EQ(1, listener.destruct);
    EXPECT_EQ(1, listener.volatile_move_assign);
  }
#endif  // POLLY_USES_STD_OPTIONAL
  EXPECT_FALSE(std::is_move_assignable<polly::optional<const int>>::value);
  EXPECT_TRUE(std::is_move_assignable<polly::optional<Copyable>>::value);
  EXPECT_TRUE(std::is_move_assignable<polly::optional<MoveableThrow>>::value);
  EXPECT_TRUE(std::is_move_assignable<polly::optional<MoveableNoThrow>>::value);
  EXPECT_FALSE(std::is_move_assignable<polly::optional<NonMovable>>::value);

  EXPECT_FALSE( std::is_nothrow_move_assignable<polly::optional<MoveableThrow>>::value);
  EXPECT_TRUE(std::is_nothrow_move_assignable<polly::optional<MoveableNoThrow>>::value);
}

struct NoConvertToOptional {
  // disable implicit conversion from const NoConvertToOptional&
  // to polly::optional<NoConvertToOptional>.
  NoConvertToOptional(const NoConvertToOptional&) = delete;
};

struct CopyConvert {
  CopyConvert(const NoConvertToOptional&);
  CopyConvert& operator=(const CopyConvert&) = delete;
  CopyConvert& operator=(const NoConvertToOptional&);
};

struct CopyConvertFromOptional {
  CopyConvertFromOptional(const NoConvertToOptional&);
  CopyConvertFromOptional(const polly::optional<NoConvertToOptional>&);
  CopyConvertFromOptional& operator=(const CopyConvertFromOptional&) = delete;
  CopyConvertFromOptional& operator=(const NoConvertToOptional&);
  CopyConvertFromOptional& operator=(
      const polly::optional<NoConvertToOptional>&);
};

struct MoveConvert {
  MoveConvert(NoConvertToOptional&&);
  MoveConvert& operator=(const MoveConvert&) = delete;
  MoveConvert& operator=(NoConvertToOptional&&);
};

struct MoveConvertFromOptional {
  MoveConvertFromOptional(NoConvertToOptional&&);
  MoveConvertFromOptional(polly::optional<NoConvertToOptional>&&);
  MoveConvertFromOptional& operator=(const MoveConvertFromOptional&) = delete;
  MoveConvertFromOptional& operator=(NoConvertToOptional&&);
  MoveConvertFromOptional& operator=(polly::optional<NoConvertToOptional>&&);
};

// template <typename U = T> polly::optional<T>& operator=(U&& v);
TEST(optionalTest, ValueAssignment) {
  polly::optional<int> opt;
  EXPECT_FALSE(opt);
  opt = 42;
  EXPECT_TRUE(opt);
  EXPECT_EQ(42, opt.value());
  opt = polly::nullopt;
  EXPECT_FALSE(opt);
  opt = 42;
  EXPECT_TRUE(opt);
  EXPECT_EQ(42, opt.value());
  opt = 43;
  EXPECT_TRUE(opt);
  EXPECT_EQ(43, opt.value());
  opt = {};  // this should clear optional
  EXPECT_FALSE(opt);

  opt = {44};
  EXPECT_TRUE(opt);
  EXPECT_EQ(44, opt.value());

  // U = const NoConvertToOptional&
  EXPECT_TRUE((std::is_assignable<polly::optional<CopyConvert>&,
                                  const NoConvertToOptional&>::value));
  // U = const polly::optional<NoConvertToOptional>&
  EXPECT_TRUE((std::is_assignable<polly::optional<CopyConvertFromOptional>&,
                                  const NoConvertToOptional&>::value));
  // U = const NoConvertToOptional& triggers SFINAE because
  // std::is_constructible_v<MoveConvert, const NoConvertToOptional&> is false
  EXPECT_FALSE((std::is_assignable<polly::optional<MoveConvert>&,
                                   const NoConvertToOptional&>::value));
  // U = NoConvertToOptional
  EXPECT_TRUE((std::is_assignable<polly::optional<MoveConvert>&,
                                  NoConvertToOptional&&>::value));
  // U = const NoConvertToOptional& triggers SFINAE because
  // std::is_constructible_v<MoveConvertFromOptional, const
  // NoConvertToOptional&> is false
  EXPECT_FALSE((std::is_assignable<polly::optional<MoveConvertFromOptional>&,
                                   const NoConvertToOptional&>::value));
  // U = NoConvertToOptional
  EXPECT_TRUE((std::is_assignable<polly::optional<MoveConvertFromOptional>&,
                                  NoConvertToOptional&&>::value));
  // U = const polly::optional<NoConvertToOptional>&
  EXPECT_TRUE((std::is_assignable<polly::optional<CopyConvertFromOptional>&,
                          const polly::optional<NoConvertToOptional>&>::value));
  // U = polly::optional<NoConvertToOptional>
  EXPECT_TRUE((std::is_assignable<polly::optional<MoveConvertFromOptional>&,
                          polly::optional<NoConvertToOptional>&&>::value));
}

// template <typename U> polly::optional<T>& operator=(const polly::optional<U>&
// rhs); template <typename U> polly::optional<T>& operator=(polly::optional<U>&&
// rhs);
TEST(optionalTest, ConvertingAssignment) {
  polly::optional<int> opt_i;
  polly::optional<char> opt_c('c');
  opt_i = opt_c;
  EXPECT_TRUE(opt_i);
  EXPECT_EQ(*opt_c, *opt_i);
  opt_i = polly::optional<char>();
  EXPECT_FALSE(opt_i);
  opt_i = polly::optional<char>('d');
  EXPECT_TRUE(opt_i);
  EXPECT_EQ('d', *opt_i);

  polly::optional<std::string> opt_str;
  polly::optional<const char*> opt_cstr("abc");
  opt_str = opt_cstr;
  EXPECT_TRUE(opt_str);
  EXPECT_EQ(std::string("abc"), *opt_str);
  opt_str = polly::optional<const char*>();
  EXPECT_FALSE(opt_str);
  opt_str = polly::optional<const char*>("def");
  EXPECT_TRUE(opt_str);
  EXPECT_EQ(std::string("def"), *opt_str);

  // operator=(const polly::optional<U>&) with U = NoConvertToOptional
  EXPECT_TRUE((std::is_assignable<polly::optional<CopyConvert>,
                                  const polly::optional<NoConvertToOptional>&>::value));
  // operator=(const polly::optional<U>&) with U = NoConvertToOptional
  // triggers SFINAE because
  // std::is_constructible_v<MoveConvert, const NoConvertToOptional&> is false
  EXPECT_FALSE((std::is_assignable<polly::optional<MoveConvert>&,
                          const polly::optional<NoConvertToOptional>&>::value));
  // operator=(polly::optional<U>&&) with U = NoConvertToOptional
  EXPECT_TRUE((std::is_assignable<polly::optional<MoveConvert>&,
                          polly::optional<NoConvertToOptional>&&>::value));
  // operator=(const polly::optional<U>&) with U = NoConvertToOptional triggers
  // SFINAE because std::is_constructible_v<MoveConvertFromOptional, const
  // NoConvertToOptional&> is false. operator=(U&&) with U = const
  // polly::optional<NoConverToOptional>& triggers SFINAE because
  // std::is_constructible<MoveConvertFromOptional,
  // polly::optional<NoConvertToOptional>&&> is true.
  EXPECT_FALSE((std::is_assignable<polly::optional<MoveConvertFromOptional>&,
                                   const polly::optional<NoConvertToOptional>&>::value));
}

TEST(optionalTest, ResetAndHasValue) {
  StructorListener listener;
  Listenable::listener = &listener;
  polly::optional<Listenable> opt;
  EXPECT_FALSE(opt);
  EXPECT_FALSE(opt.has_value());
  opt.emplace();
  EXPECT_TRUE(opt);
  EXPECT_TRUE(opt.has_value());
  opt.reset();
  EXPECT_FALSE(opt);
  EXPECT_FALSE(opt.has_value());
  EXPECT_EQ(1, listener.destruct);
  opt.reset();
  EXPECT_FALSE(opt);
  EXPECT_FALSE(opt.has_value());

  constexpr polly::optional<int> empty;
  static_assert(!empty.has_value(), "");
  constexpr polly::optional<int> nonempty(1);
  static_assert(nonempty.has_value(), "");
}

TEST(optionalTest, Emplace) {
  StructorListener listener;
  Listenable::listener = &listener;
  polly::optional<Listenable> opt;
  EXPECT_FALSE(opt);
  opt.emplace(1);
  EXPECT_TRUE(opt);
  opt.emplace(1, 2);
  EXPECT_EQ(1, listener.construct1);
  EXPECT_EQ(1, listener.construct2);
  EXPECT_EQ(1, listener.destruct);

  polly::optional<std::string> o;
  EXPECT_TRUE((std::is_same<std::string&, decltype(o.emplace("abc"))>::value));
  std::string& ref = o.emplace("abc");
  EXPECT_EQ(&ref, &o.value());
}

TEST(optionalTest, ListEmplace) {
  StructorListener listener;
  Listenable::listener = &listener;
  polly::optional<Listenable> opt;
  EXPECT_FALSE(opt);
  opt.emplace({1});
  EXPECT_TRUE(opt);
  opt.emplace({1, 2});
  EXPECT_EQ(2, listener.listinit);
  EXPECT_EQ(1, listener.destruct);

  polly::optional<Listenable> o;
  EXPECT_TRUE((std::is_same<Listenable&, decltype(o.emplace({1}))>::value));
  Listenable& ref = o.emplace({1});
  EXPECT_EQ(&ref, &o.value());
}

TEST(optionalTest, Swap) {
  polly::optional<int> opt_empty, opt1 = 1, opt2 = 2;
  EXPECT_FALSE(opt_empty);
  EXPECT_TRUE(opt1);
  EXPECT_EQ(1, opt1.value());
  EXPECT_TRUE(opt2);
  EXPECT_EQ(2, opt2.value());
  swap(opt_empty, opt1);
  EXPECT_FALSE(opt1);
  EXPECT_TRUE(opt_empty);
  EXPECT_EQ(1, opt_empty.value());
  EXPECT_TRUE(opt2);
  EXPECT_EQ(2, opt2.value());
  swap(opt_empty, opt1);
  EXPECT_FALSE(opt_empty);
  EXPECT_TRUE(opt1);
  EXPECT_EQ(1, opt1.value());
  EXPECT_TRUE(opt2);
  EXPECT_EQ(2, opt2.value());
  swap(opt1, opt2);
  EXPECT_FALSE(opt_empty);
  EXPECT_TRUE(opt1);
  EXPECT_EQ(2, opt1.value());
  EXPECT_TRUE(opt2);
  EXPECT_EQ(1, opt2.value());

  EXPECT_TRUE(noexcept(opt1.swap(opt2)));
  EXPECT_TRUE(noexcept(swap(opt1, opt2)));
}

template <int v>
struct DeletedOpAddr {
  int value = v;
  constexpr DeletedOpAddr() = default;
  constexpr const DeletedOpAddr<v>* operator&() const = delete;  // NOLINT
  DeletedOpAddr<v>* operator&() = delete;                        // NOLINT
};

// The static_assert featuring a constexpr call to operator->() is commented out
// to document the fact that the current implementation of polly::optional<T>
// expects such usecases to be malformed and not compile.
TEST(optionalTest, OperatorAddr) {
  constexpr int v = -1;
  {  // constexpr
    constexpr polly::optional<DeletedOpAddr<v>> opt(polly::in_place_t{});
    static_assert(opt.has_value(), "");
    // static_assert(opt->value == v, "");
    static_assert((*opt).value == v, "");
  }
  {  // non-constexpr
    const polly::optional<DeletedOpAddr<v>> opt(polly::in_place_t{});
    EXPECT_TRUE(opt.has_value());
    EXPECT_TRUE(opt->value == v);
    EXPECT_TRUE((*opt).value == v);
  }
}

TEST(optionalTest, PointerStuff) {
  polly::optional<std::string> opt(polly::in_place, "foo");
  EXPECT_EQ("foo", *opt);
  const auto& opt_const = opt;
  EXPECT_EQ("foo", *opt_const);
  EXPECT_EQ(opt->size(), 3);
  EXPECT_EQ(opt_const->size(), 3);

  constexpr polly::optional<ConstexprType> opt1(1);
  static_assert((*opt1).x == ConstexprType::kCtorInt, "");
}

// gcc has a bug pre 4.9.1 where it doesn't do correct overload resolution
// when overloads are const-qualified and *this is an raluve.
// Skip that test to make the build green again when using the old compiler.
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=59296 is fixed in 4.9.1.
#if defined(__GNUC__) && !defined(__clang__)
#define GCC_VERSION (__GNUC__ * 10000 \
                     + __GNUC_MINOR__ * 100 \
                     + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40901
#define POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
#endif
#endif

// MSVC has a bug with "cv-qualifiers in class construction", fixed in 2017. See
// https://docs.microsoft.com/en-us/cpp/cpp-conformance-improvements-2017#bug-fixes
// The compiler some incorrectly ingores the cv-qualifier when generating a
// class object via a constructor call. For example:
//
// class optional {
//   constexpr T&& value() &&;
//   constexpr const T&& value() const &&;
// }
//
// using COI = const polly::optional<int>;
// static_assert(2 == COI(2).value(), "");  // const &&
//
// This should invoke the "const &&" overload but since it ignores the const
// qualifier it finds the "&&" overload the best candidate.
#if defined(_MSC_VER) && _MSC_VER < 1910
#define POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
#endif

TEST(optionalTest, Value) {
  using O = polly::optional<std::string>;
  using CO = const polly::optional<std::string>;
  using OC = polly::optional<const std::string>;
  O lvalue(polly::in_place, "lvalue");
  CO clvalue(polly::in_place, "clvalue");
  OC lvalue_c(polly::in_place, "lvalue_c");
  EXPECT_EQ("lvalue", lvalue.value());
  EXPECT_EQ("clvalue", clvalue.value());
  EXPECT_EQ("lvalue_c", lvalue_c.value());
  EXPECT_EQ("xvalue", O(polly::in_place, "xvalue").value());
  EXPECT_EQ("xvalue_c", OC(polly::in_place, "xvalue_c").value());
#ifndef POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
  EXPECT_EQ("cxvalue", CO(polly::in_place, "cxvalue").value());
#endif
  EXPECT_EQ("&", TypeQuals(lvalue.value()));
  EXPECT_EQ("c&", TypeQuals(clvalue.value()));
  EXPECT_EQ("c&", TypeQuals(lvalue_c.value()));
  EXPECT_EQ("&&", TypeQuals(O(polly::in_place, "xvalue").value()));
#if !defined(POLLY_SKIP_OVERLOAD_TEST_DUE_TO_MSVC_BUG) && \
    !defined(POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG)
  EXPECT_EQ("c&&", TypeQuals(CO(polly::in_place, "cxvalue").value()));
#endif
  EXPECT_EQ("c&&", TypeQuals(OC(polly::in_place, "xvalue_c").value()));

  // test on volatile type
  using OV = polly::optional<volatile int>;
  OV lvalue_v(polly::in_place, 42);
  EXPECT_EQ(42, lvalue_v.value());
  EXPECT_EQ(42, OV(42).value());
  EXPECT_TRUE((std::is_same<volatile int&, decltype(lvalue_v.value())>::value));
  EXPECT_TRUE((std::is_same<volatile int&&, decltype(OV(42).value())>::value));

  // test exception throw on value()
  polly::optional<int> empty;
#ifdef POLLY_HAVE_EXCEPTIONS
  EXPECT_THROW((void)empty.value(), polly::bad_optional_access);
#else
  EXPECT_DEATH_IF_SUPPORTED((void)empty.value(), "Bad optional access");
#endif

  // test constexpr value()
  constexpr polly::optional<int> o1(1);
  static_assert(1 == o1.value(), "");  // const &
#if !defined(_MSC_VER) && !defined(POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG)
  using COI = const polly::optional<int>;
  static_assert(2 == COI(2).value(), "");  // const &&
#endif
}

TEST(optionalTest, DerefOperator) {
  using O = polly::optional<std::string>;
  using CO = const polly::optional<std::string>;
  using OC = polly::optional<const std::string>;
  O lvalue(polly::in_place, "lvalue");
  CO clvalue(polly::in_place, "clvalue");
  OC lvalue_c(polly::in_place, "lvalue_c");
  EXPECT_EQ("lvalue", *lvalue);
  EXPECT_EQ("clvalue", *clvalue);
  EXPECT_EQ("lvalue_c", *lvalue_c);
  EXPECT_EQ("xvalue", *O(polly::in_place, "xvalue"));
  EXPECT_EQ("xvalue_c", *OC(polly::in_place, "xvalue_c"));
#ifndef POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG
  EXPECT_EQ("cxvalue", *CO(polly::in_place, "cxvalue"));
#endif
  EXPECT_EQ("&", TypeQuals(*lvalue));
  EXPECT_EQ("c&", TypeQuals(*clvalue));
  EXPECT_EQ("&&", TypeQuals(*O(polly::in_place, "xvalue")));
#if !defined(POLLY_SKIP_OVERLOAD_TEST_DUE_TO_MSVC_BUG) && \
    !defined(POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG)
  EXPECT_EQ("c&&", TypeQuals(*CO(polly::in_place, "cxvalue")));
#endif
  EXPECT_EQ("c&&", TypeQuals(*OC(polly::in_place, "xvalue_c")));

  // test on volatile type
  using OV = polly::optional<volatile int>;
  OV lvalue_v(polly::in_place, 42);
  EXPECT_EQ(42, *lvalue_v);
  EXPECT_EQ(42, *OV(42));
  EXPECT_TRUE((std::is_same<volatile int&, decltype(*lvalue_v)>::value));
  EXPECT_TRUE((std::is_same<volatile int&&, decltype(*OV(42))>::value));

  constexpr polly::optional<int> opt1(1);
  static_assert(*opt1 == 1, "");
#if !defined(_MSC_VER) && !defined(POLLY_SKIP_OVERLOAD_TEST_DUE_TO_GCC_BUG)
  using COI = const polly::optional<int>;
  static_assert(*COI(2) == 2, "");
#endif
}

TEST(optionalTest, ValueOr) {
  polly::optional<double> opt_empty, opt_set = 1.2;
  EXPECT_EQ(42.0, opt_empty.value_or(42));
  EXPECT_EQ(1.2, opt_set.value_or(42));
  EXPECT_EQ(42.0, polly::optional<double>().value_or(42));
  EXPECT_EQ(1.2, polly::optional<double>(1.2).value_or(42));

  constexpr polly::optional<double> copt_empty, copt_set = 1.2;
  static_assert(42.0 == copt_empty.value_or(42), "");
  static_assert(1.2 == copt_set.value_or(42), "");
#ifndef POLLY_SKIP_OVERLOAD_TEST_DUE_TO_MSVC_BUG
  using COD = const polly::optional<double>;
  static_assert(42 == COD().value_or(42), "");
  static_assert(1.2 == COD(1.2).value_or(42), "");
#endif
}

// make_optional cannot be constexpr until C++17
TEST(optionalTest, make_optional) {
  auto opt_int = polly::make_optional(42);
  EXPECT_TRUE((std::is_same<decltype(opt_int), polly::optional<int>>::value));
  EXPECT_EQ(42, opt_int);

  StructorListener listener;
  Listenable::listener = &listener;

  polly::optional<Listenable> opt0 = polly::make_optional<Listenable>();
  EXPECT_EQ(1, listener.construct0);
  polly::optional<Listenable> opt1 = polly::make_optional<Listenable>(1);
  EXPECT_EQ(1, listener.construct1);
  polly::optional<Listenable> opt2 = polly::make_optional<Listenable>(1, 2);
  EXPECT_EQ(1, listener.construct2);
  polly::optional<Listenable> opt3 = polly::make_optional<Listenable>({1});
  polly::optional<Listenable> opt4 = polly::make_optional<Listenable>({1, 2});
  EXPECT_EQ(2, listener.listinit);

  // Constexpr tests on trivially copyable types
  // optional<T> has trivial copy/move ctors when T is trivially copyable.
  // For nontrivial types with constexpr constructors, we need copy elision in
  // C++17 for make_optional to be constexpr.
  {
    constexpr polly::optional<int> c_opt = polly::make_optional(42);
    static_assert(c_opt.value() == 42, "");
  }
  {
    struct TrivialCopyable {
      constexpr TrivialCopyable() : x(0) {}
      constexpr explicit TrivialCopyable(int i) : x(i) {}
      int x;
    };

    constexpr TrivialCopyable v;
    constexpr polly::optional<TrivialCopyable> c_opt0 = polly::make_optional(v);
    static_assert((*c_opt0).x == 0, "");
    constexpr polly::optional<TrivialCopyable> c_opt1 =
        polly::make_optional<TrivialCopyable>();
    static_assert((*c_opt1).x == 0, "");
    constexpr polly::optional<TrivialCopyable> c_opt2 =
        polly::make_optional<TrivialCopyable>(42);
    static_assert((*c_opt2).x == 42, "");
  }
}

template <typename T, typename U>
void optionalTest_Comparisons_EXPECT_LESS(T x, U y) {
  EXPECT_FALSE(x == y);
  EXPECT_TRUE(x != y);
  EXPECT_TRUE(x < y);
  EXPECT_FALSE(x > y);
  EXPECT_TRUE(x <= y);
  EXPECT_FALSE(x >= y);
}

template <typename T, typename U>
void optionalTest_Comparisons_EXPECT_SAME(T x, U y) {
  EXPECT_TRUE(x == y);
  EXPECT_FALSE(x != y);
  EXPECT_FALSE(x < y);
  EXPECT_FALSE(x > y);
  EXPECT_TRUE(x <= y);
  EXPECT_TRUE(x >= y);
}

template <typename T, typename U>
void optionalTest_Comparisons_EXPECT_GREATER(T x, U y) {
  EXPECT_FALSE(x == y);
  EXPECT_TRUE(x != y);
  EXPECT_FALSE(x < y);
  EXPECT_TRUE(x > y);
  EXPECT_FALSE(x <= y);
  EXPECT_TRUE(x >= y);
}


template <typename T, typename U, typename V>
void TestComparisons() {
  polly::optional<T> ae, a2(2), a4(4);
  polly::optional<U> be, b2(2), b4(4);
  V v3 = 3;

  // LHS: polly::nullopt, ae, a2, v3, a4
  // RHS: polly::nullopt, be, b2, v3, b4

  // optionalTest_Comparisons_EXPECT_NOT_TO_WORK(polly::nullopt,polly::nullopt);
  optionalTest_Comparisons_EXPECT_SAME(polly::nullopt, be);
  optionalTest_Comparisons_EXPECT_LESS(polly::nullopt, b2);
  // optionalTest_Comparisons_EXPECT_NOT_TO_WORK(polly::nullopt,v3);
  optionalTest_Comparisons_EXPECT_LESS(polly::nullopt, b4);

  optionalTest_Comparisons_EXPECT_SAME(ae, polly::nullopt);
  optionalTest_Comparisons_EXPECT_SAME(ae, be);
  optionalTest_Comparisons_EXPECT_LESS(ae, b2);
  optionalTest_Comparisons_EXPECT_LESS(ae, v3);
  optionalTest_Comparisons_EXPECT_LESS(ae, b4);

  optionalTest_Comparisons_EXPECT_GREATER(a2, polly::nullopt);
  optionalTest_Comparisons_EXPECT_GREATER(a2, be);
  optionalTest_Comparisons_EXPECT_SAME(a2, b2);
  optionalTest_Comparisons_EXPECT_LESS(a2, v3);
  optionalTest_Comparisons_EXPECT_LESS(a2, b4);

  // optionalTest_Comparisons_EXPECT_NOT_TO_WORK(v3,polly::nullopt);
  optionalTest_Comparisons_EXPECT_GREATER(v3, be);
  optionalTest_Comparisons_EXPECT_GREATER(v3, b2);
  optionalTest_Comparisons_EXPECT_SAME(v3, v3);
  optionalTest_Comparisons_EXPECT_LESS(v3, b4);

  optionalTest_Comparisons_EXPECT_GREATER(a4, polly::nullopt);
  optionalTest_Comparisons_EXPECT_GREATER(a4, be);
  optionalTest_Comparisons_EXPECT_GREATER(a4, b2);
  optionalTest_Comparisons_EXPECT_GREATER(a4, v3);
  optionalTest_Comparisons_EXPECT_SAME(a4, b4);
}

struct Int1 {
  Int1() = default;
  Int1(int i) : i(i) {}  // NOLINT(runtime/explicit)
  int i;
};

struct Int2 {
  Int2() = default;
  Int2(int i) : i(i) {}  // NOLINT(runtime/explicit)
  int i;
};

// comparison between Int1 and Int2
//constexpr bool operator==(const Int1& lhs, const Int2& rhs) {
//  return lhs.i == rhs.i;
//}
constexpr bool operator==(const Int1& lhs, const Int2& rhs) {
  return lhs.i == rhs.i;
}
constexpr bool operator==(const Int2& lhs, const Int1& rhs) {
  return lhs.i == rhs.i;
}
constexpr bool operator!=(const Int1& lhs, const Int2& rhs) {
  return !(lhs == rhs);
}
constexpr bool operator!=(const Int2& lhs, const Int1& rhs) {
  return !(lhs == rhs);
}
constexpr bool operator<(const Int1& lhs, const Int2& rhs) {
  return lhs.i < rhs.i;
}
constexpr bool operator<(const Int2& lhs, const Int1& rhs) {
  return lhs.i < rhs.i;
}
constexpr bool operator<=(const Int1& lhs, const Int2& rhs) {
  return lhs < rhs || lhs == rhs;
}
constexpr bool operator<=(const Int2& lhs, const Int1& rhs) {
  return lhs < rhs || lhs == rhs;
}
constexpr bool operator>(const Int1& lhs, const Int2& rhs) {
  return !(lhs <= rhs);
}
constexpr bool operator>(const Int2& lhs, const Int1& rhs) {
  return !(lhs <= rhs);
}
constexpr bool operator>=(const Int1& lhs, const Int2& rhs) {
  return !(lhs < rhs);
}
constexpr bool operator>=(const Int2& lhs, const Int1& rhs) {
  return !(lhs < rhs);
}

TEST(optionalTest, Comparisons) {
  TestComparisons<int, int, int>();
  TestComparisons<const int, int, int>();
  TestComparisons<Int1, int, int>();
  TestComparisons<int, Int2, int>();
  TestComparisons<Int1, Int2, int>();

  // compare polly::optional<std::string> with const char*
  polly::optional<std::string> opt_str = "abc";
  const char* cstr = "abc";
  EXPECT_TRUE(opt_str == cstr);
  // compare polly::optional<std::string> with polly::optional<const char*>
  polly::optional<const char*> opt_cstr = cstr;
  EXPECT_TRUE(opt_str == opt_cstr);
  // compare polly::optional<std::string> with polly::optional<polly::string_view>
  polly::optional<polly::string_view> e1;
  polly::optional<std::string> e2;
  EXPECT_TRUE(e1 == e2);
}

TEST(optionalTest, SwapRegression) {
  StructorListener listener;
  Listenable::listener = &listener;

  {
    polly::optional<Listenable> a;
    polly::optional<Listenable> b(polly::in_place);
    a.swap(b);
  }

  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(1, listener.move);
  EXPECT_EQ(2, listener.destruct);

  {
    polly::optional<Listenable> a(polly::in_place);
    polly::optional<Listenable> b;
    a.swap(b);
  }

  EXPECT_EQ(2, listener.construct0);
  EXPECT_EQ(2, listener.move);
  EXPECT_EQ(4, listener.destruct);
}

TEST(optionalTest, BigStringLeakCheck) {
  constexpr size_t n = 1 << 16;

  using OS = polly::optional<std::string>;

  OS a;
  OS b = polly::nullopt;
  OS c = std::string(n, 'c');
  std::string sd(n, 'd');
  OS d = sd;
  OS e(polly::in_place, n, 'e');
  OS f;
  f.emplace(n, 'f');

  OS ca(a);
  OS cb(b);
  OS cc(c);
  OS cd(d);
  OS ce(e);

  OS oa;
  OS ob = polly::nullopt;
  OS oc = std::string(n, 'c');
  std::string sod(n, 'd');
  OS od = sod;
  OS oe(polly::in_place, n, 'e');
  OS of;
  of.emplace(n, 'f');

  OS ma(std::move(oa));
  OS mb(std::move(ob));
  OS mc(std::move(oc));
  OS md(std::move(od));
  OS me(std::move(oe));
  OS mf(std::move(of));

  OS aa1;
  OS ab1 = polly::nullopt;
  OS ac1 = std::string(n, 'c');
  std::string sad1(n, 'd');
  OS ad1 = sad1;
  OS ae1(polly::in_place, n, 'e');
  OS af1;
  af1.emplace(n, 'f');

  OS aa2;
  OS ab2 = polly::nullopt;
  OS ac2 = std::string(n, 'c');
  std::string sad2(n, 'd');
  OS ad2 = sad2;
  OS ae2(polly::in_place, n, 'e');
  OS af2;
  af2.emplace(n, 'f');

  aa1 = af2;
  ab1 = ae2;
  ac1 = ad2;
  ad1 = ac2;
  ae1 = ab2;
  af1 = aa2;

  OS aa3;
  OS ab3 = polly::nullopt;
  OS ac3 = std::string(n, 'c');
  std::string sad3(n, 'd');
  OS ad3 = sad3;
  OS ae3(polly::in_place, n, 'e');
  OS af3;
  af3.emplace(n, 'f');

  aa3 = polly::nullopt;
  ab3 = polly::nullopt;
  ac3 = polly::nullopt;
  ad3 = polly::nullopt;
  ae3 = polly::nullopt;
  af3 = polly::nullopt;

  OS aa4;
  OS ab4 = polly::nullopt;
  OS ac4 = std::string(n, 'c');
  std::string sad4(n, 'd');
  OS ad4 = sad4;
  OS ae4(polly::in_place, n, 'e');
  OS af4;
  af4.emplace(n, 'f');

  aa4 = OS(polly::in_place, n, 'a');
  ab4 = OS(polly::in_place, n, 'b');
  ac4 = OS(polly::in_place, n, 'c');
  ad4 = OS(polly::in_place, n, 'd');
  ae4 = OS(polly::in_place, n, 'e');
  af4 = OS(polly::in_place, n, 'f');

  OS aa5;
  OS ab5 = polly::nullopt;
  OS ac5 = std::string(n, 'c');
  std::string sad5(n, 'd');
  OS ad5 = sad5;
  OS ae5(polly::in_place, n, 'e');
  OS af5;
  af5.emplace(n, 'f');

  std::string saa5(n, 'a');
  std::string sab5(n, 'a');
  std::string sac5(n, 'a');
  std::string sad52(n, 'a');
  std::string sae5(n, 'a');
  std::string saf5(n, 'a');

  aa5 = saa5;
  ab5 = sab5;
  ac5 = sac5;
  ad5 = sad52;
  ae5 = sae5;
  af5 = saf5;

  OS aa6;
  OS ab6 = polly::nullopt;
  OS ac6 = std::string(n, 'c');
  std::string sad6(n, 'd');
  OS ad6 = sad6;
  OS ae6(polly::in_place, n, 'e');
  OS af6;
  af6.emplace(n, 'f');

  aa6 = std::string(n, 'a');
  ab6 = std::string(n, 'b');
  ac6 = std::string(n, 'c');
  ad6 = std::string(n, 'd');
  ae6 = std::string(n, 'e');
  af6 = std::string(n, 'f');

  OS aa7;
  OS ab7 = polly::nullopt;
  OS ac7 = std::string(n, 'c');
  std::string sad7(n, 'd');
  OS ad7 = sad7;
  OS ae7(polly::in_place, n, 'e');
  OS af7;
  af7.emplace(n, 'f');

  aa7.emplace(n, 'A');
  ab7.emplace(n, 'B');
  ac7.emplace(n, 'C');
  ad7.emplace(n, 'D');
  ae7.emplace(n, 'E');
  af7.emplace(n, 'F');
}

TEST(optionalTest, MoveAssignRegression) {
  StructorListener listener;
  Listenable::listener = &listener;

  {
    polly::optional<Listenable> a;
    Listenable b;
    a = std::move(b);
  }

  EXPECT_EQ(1, listener.construct0);
  EXPECT_EQ(1, listener.move);
  EXPECT_EQ(2, listener.destruct);
}

TEST(optionalTest, ValueType) {
  EXPECT_TRUE((std::is_same<polly::optional<int>::value_type, int>::value));
  EXPECT_TRUE((std::is_same<polly::optional<std::string>::value_type,
                            std::string>::value));
  EXPECT_FALSE((std::is_same<polly::optional<int>::value_type, polly::nullopt_t>::value));
}

template <typename T>
struct is_hash_enabled_for {
  template <typename U, typename = decltype(std::hash<U>()(std::declval<U>()))>
  static std::true_type test(int);

  template <typename U>
  static std::false_type test(...);

  static constexpr bool value = decltype(test<T>(0))::value;
};

TEST(optionalTest, Hash) {
  std::hash<polly::optional<int>> hash;
  std::set<size_t> hashcodes;
  hashcodes.insert(hash(polly::nullopt));
  for (int i = 0; i < 100; ++i) {
    hashcodes.insert(hash(i));
  }
  EXPECT_GT(hashcodes.size(), 90);

  static_assert(is_hash_enabled_for<polly::optional<int>>::value, "");
  static_assert(is_hash_enabled_for<polly::optional<Hashable>>::value, "");
  static_assert(is_hash_enabled_for<polly::optional<int>>::value, "");
  static_assert(is_hash_enabled_for<polly::optional<Hashable>>::value, "");
  //polly::type_traits_internal::AssertHashEnabled<polly::optional<int>>();
  //polly::type_traits_internal::AssertHashEnabled<polly::optional<Hashable>>();

  static_assert(!is_hash_enabled_for<polly::optional<NonHashable>>::value, "");
  static_assert(!is_hash_enabled_for<polly::optional<NonHashable>>::value, "");

  // libstdc++ std::optional is missing remove_const_t, i.e. it's using
  // std::hash<T> rather than std::hash<std::remove_const_t<T>>.
  // Reference: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=82262
#ifndef __GLIBCXX__
  static_assert(is_hash_enabled_for<polly::optional<const int>>::value, "");
  static_assert(is_hash_enabled_for<polly::optional<const Hashable>>::value, "");
  std::hash<polly::optional<const int>> c_hash;
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(hash(i), c_hash(i));
  }
#endif
}

struct MoveMeNoThrow {
  MoveMeNoThrow() : x(0) {}
  MoveMeNoThrow(const MoveMeNoThrow& other) noexcept : x(other.x) {}
  MoveMeNoThrow(MoveMeNoThrow&& other) noexcept : x(other.x) {}
  MoveMeNoThrow& operator=(MoveMeNoThrow&& o) noexcept { x = o.x; return *this; }
  int x;
};

struct MoveMeThrow {
  MoveMeThrow() : x(0) {}
  MoveMeThrow(const MoveMeThrow& other) : x(other.x) {}
  MoveMeThrow(MoveMeThrow&& other) : x(other.x) {}
  int x;
};

TEST(optionalTest, NoExcept) {
  static_assert(std::is_nothrow_move_constructible<polly::optional<MoveMeNoThrow>>::value, "");
  static_assert(std::is_nothrow_move_assignable<polly::optional<MoveMeNoThrow>>::value, "");
  static_assert(!std::is_nothrow_move_constructible<polly::optional<MoveMeThrow>>::value, "");
  std::vector<polly::optional<MoveMeNoThrow>> v;
  for (int i = 0; i < 10; ++i) v.emplace_back();
}

struct AnyLike {
  AnyLike(AnyLike&&) = default;
  AnyLike(const AnyLike&) = default;

  template <
      typename ValueType,
      typename T = typename std::decay<ValueType>::type,
      polly::Requires<
          polly::Not<
              polly::Or<
                std::is_same<AnyLike, T>,
                polly::Not<std::is_copy_constructible<T>>
              >
          >
      > = true
  >
  AnyLike(ValueType&&) {}  // NOLINT(runtime/explicit)

  AnyLike& operator=(AnyLike&&) = default;
  AnyLike& operator=(const AnyLike&) = default;

  template <typename ValueType,
      typename T = typename std::decay<ValueType>::type>
  typename std::enable_if<
      polly::And<
          polly::Not<std::is_same<AnyLike, T>>,
          std::is_copy_constructible<T>
      >::value,
      AnyLike&
  >::type
  operator=(ValueType&& /* rhs */) {
    return *this;
  }
};

TEST(optionalTest, ConstructionConstraints) {
  EXPECT_TRUE((std::is_constructible<AnyLike, polly::optional<AnyLike>>::value));
  // EXPECT_TRUE((std::is_constructible<AnyLike, const polly::optional<AnyLike>&>::value));
  EXPECT_TRUE((std::is_constructible<polly::optional<AnyLike>, AnyLike>::value));
  EXPECT_TRUE((std::is_constructible<polly::optional<AnyLike>, const AnyLike&>::value));
  EXPECT_TRUE((std::is_convertible<polly::optional<AnyLike>, AnyLike>::value));
  // EXPECT_TRUE((std::is_convertible<const polly::optional<AnyLike>&, AnyLike>::value));
  EXPECT_TRUE((std::is_convertible<AnyLike, polly::optional<AnyLike>>::value));
  EXPECT_TRUE( (std::is_convertible<const AnyLike&, polly::optional<AnyLike>>::value));
  EXPECT_TRUE(std::is_move_constructible<polly::optional<AnyLike>>::value);
  EXPECT_TRUE(std::is_copy_constructible<polly::optional<AnyLike>>::value);
}

TEST(optionalTest, AssignmentConstraints) {
  EXPECT_TRUE((std::is_assignable<AnyLike&, polly::optional<AnyLike>>::value));
  EXPECT_TRUE((std::is_assignable<AnyLike&, const polly::optional<AnyLike>&>::value));
  EXPECT_TRUE((std::is_assignable<polly::optional<AnyLike>&, AnyLike>::value));
  EXPECT_TRUE((std::is_assignable<polly::optional<AnyLike>&, const AnyLike&>::value));
  EXPECT_TRUE(std::is_move_assignable<polly::optional<AnyLike>>::value);
  EXPECT_TRUE(std::is_copy_assignable<polly::optional<AnyLike>>::value);
}

struct NestedClassBug {
  struct Inner {
    bool dummy = false;
  };
  polly::optional<Inner> value;
};

TEST(optionalTest, InPlaceTSFINAEBug) {
  NestedClassBug b;
  (void)b;
  using Inner = NestedClassBug::Inner;

  EXPECT_TRUE((std::is_default_constructible<Inner>::value));
  EXPECT_TRUE((std::is_constructible<Inner>::value));
  EXPECT_TRUE(
      (std::is_constructible<polly::optional<Inner>, polly::in_place_t>::value));

  polly::optional<Inner> o(polly::in_place);
  EXPECT_TRUE(o.has_value());
  o.emplace();
  EXPECT_TRUE(o.has_value());
}

}  // namespace

#endif  // POLLY_HAVE_STD_OPTIONAL
