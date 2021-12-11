#include "stubs/type_traits.h"

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "gtest/gtest.h"

namespace {
using ::testing::StaticAssertTypeEq;

template <typename First, typename Second>
struct Pair {
  First first;
  Second second;
};

struct Dummy {};

struct StructA {};
struct StructB {};
struct StructC {};

TEST(VoidTTest, BasicUsage) {
  StaticAssertTypeEq<void, polly::void_t<Dummy>>();
  StaticAssertTypeEq<void, polly::void_t<Dummy, Dummy, Dummy>>();
}

TEST(AndTest, BasicBooleanLogic) {
  EXPECT_TRUE(polly::And<>::value);
  EXPECT_TRUE(polly::And<std::true_type>::value);
  EXPECT_TRUE((polly::And<std::true_type, std::true_type>::value));
  EXPECT_FALSE((polly::And<std::true_type, std::false_type>::value));
  EXPECT_FALSE((polly::And<std::false_type, std::true_type>::value));
  EXPECT_FALSE((polly::And<std::false_type, std::false_type>::value));
}

struct TrueType {
  static constexpr bool value = true;
};

struct FalseType {
  static constexpr bool value = false;
};

TEST(AndTest, ShortCircuiting) {
  EXPECT_FALSE((polly::And<std::true_type, std::false_type, Dummy>::value));
  EXPECT_TRUE((std::is_base_of<FalseType, polly::And<std::true_type, FalseType, std::false_type>>::value));
  EXPECT_TRUE((std::is_base_of<TrueType, polly::And<std::true_type, TrueType>>::value));
}

TEST(OrTest, BasicBooleanLogic) {
  EXPECT_FALSE(polly::Or<>::value);
  EXPECT_FALSE(polly::Or<std::false_type>::value);
  EXPECT_TRUE((polly::Or<std::true_type, std::true_type>::value));
  EXPECT_TRUE((polly::Or<std::true_type, std::false_type>::value));
  EXPECT_TRUE((polly::Or<std::false_type, std::true_type>::value));
  EXPECT_FALSE((polly::Or<std::false_type, std::false_type>::value));
}

TEST(OrTest, ShortCircuiting) {
  EXPECT_TRUE((polly::Or<std::false_type, std::true_type, Dummy>::value));
  EXPECT_TRUE((std::is_base_of<TrueType, polly::Or<std::false_type, TrueType, std::true_type>>::value));
  EXPECT_TRUE((std::is_base_of<FalseType, polly::Or<std::false_type, FalseType>>::value));
}

TEST(NotTest, BasicBooleanLogic) {
  EXPECT_FALSE(polly::Not<std::true_type>::value);
  EXPECT_FALSE(polly::Not<TrueType>::value);
  EXPECT_TRUE(polly::Not<std::false_type>::value);
  EXPECT_TRUE(polly::Not<FalseType>::value);
}

// all member functions are trivial
class Trivial {
  int n_;
};

struct TrivialDestructor {
  ~TrivialDestructor() = default;
};

struct NontrivialDestructor {
  ~NontrivialDestructor() {}
};

struct DeletedDestructor {
  ~DeletedDestructor() = delete;
};

class TrivialDefaultCtor {
 public:
  TrivialDefaultCtor() = default;
  explicit TrivialDefaultCtor(int n) : n_(n) {}

 private:
  int n_;
};

class NontrivialDefaultCtor {
 public:
  NontrivialDefaultCtor() : n_(1) {}

 private:
  int n_;
};

class DeletedDefaultCtor {
 public:
  DeletedDefaultCtor() = delete;
  explicit DeletedDefaultCtor(int n) : n_(n) {}

 private:
  int n_;
};

class TrivialMoveCtor {
 public:
  explicit TrivialMoveCtor(int n) : n_(n) {}
  TrivialMoveCtor(TrivialMoveCtor&&) = default;
  TrivialMoveCtor& operator=(const TrivialMoveCtor& t) {
    n_ = t.n_;
    return *this;
  }

 private:
  int n_;
};

class NontrivialMoveCtor {
 public:
  explicit NontrivialMoveCtor(int n) : n_(n) {}
  NontrivialMoveCtor(NontrivialMoveCtor&& t) noexcept : n_(t.n_) {}
  NontrivialMoveCtor& operator=(const NontrivialMoveCtor&) = default;

 private:
  int n_;
};

class TrivialCopyCtor {
 public:
  explicit TrivialCopyCtor(int n) : n_(n) {}
  TrivialCopyCtor(const TrivialCopyCtor&) = default;
  TrivialCopyCtor& operator=(const TrivialCopyCtor& t) {
    n_ = t.n_;
    return *this;
  }

 private:
  int n_;
};

class NontrivialCopyCtor {
 public:
  explicit NontrivialCopyCtor(int n) : n_(n) {}
  NontrivialCopyCtor(const NontrivialCopyCtor& t) : n_(t.n_) {}
  NontrivialCopyCtor& operator=(const NontrivialCopyCtor&) = default;

 private:
  int n_;
};

class DeletedCopyCtor {
 public:
  explicit DeletedCopyCtor(int n) : n_(n) {}
  DeletedCopyCtor(const DeletedCopyCtor&) = delete;
  DeletedCopyCtor& operator=(const DeletedCopyCtor&) = default;

 private:
  int n_;
};

class TrivialMoveAssign {
 public:
  explicit TrivialMoveAssign(int n) : n_(n) {}
  TrivialMoveAssign(const TrivialMoveAssign& t) : n_(t.n_) {}
  TrivialMoveAssign& operator=(TrivialMoveAssign&&) = default;
  ~TrivialMoveAssign() {}  // can have nontrivial destructor
 private:
  int n_;
};

class NontrivialMoveAssign {
 public:
  explicit NontrivialMoveAssign(int n) : n_(n) {}
  NontrivialMoveAssign(const NontrivialMoveAssign&) = default;
  NontrivialMoveAssign& operator=(NontrivialMoveAssign&& t) noexcept {
    n_ = t.n_;
    return *this;
  }

 private:
  int n_;
};

class TrivialCopyAssign {
 public:
  explicit TrivialCopyAssign(int n) : n_(n) {}
  TrivialCopyAssign(const TrivialCopyAssign& t) : n_(t.n_) {}
  TrivialCopyAssign& operator=(const TrivialCopyAssign& t) = default;
  ~TrivialCopyAssign() {}  // can have nontrivial destructor
 private:
  int n_;
};

class NontrivialCopyAssign {
 public:
  explicit NontrivialCopyAssign(int n) : n_(n) {}
  NontrivialCopyAssign(const NontrivialCopyAssign&) = default;
  NontrivialCopyAssign& operator=(const NontrivialCopyAssign& t) {
    n_ = t.n_;
    return *this;
  }

 private:
  int n_;
};

class DeletedCopyAssign {
 public:
  explicit DeletedCopyAssign(int n) : n_(n) {}
  DeletedCopyAssign(const DeletedCopyAssign&) = default;
  DeletedCopyAssign& operator=(const DeletedCopyAssign&) = delete;

 private:
  int n_;
};

struct MovableNonCopyable {
  MovableNonCopyable() = default;
  MovableNonCopyable(const MovableNonCopyable&) = delete;
  MovableNonCopyable(MovableNonCopyable&&) = default;
  MovableNonCopyable& operator=(const MovableNonCopyable&) = delete;
  MovableNonCopyable& operator=(MovableNonCopyable&&) = default;
};

struct NonCopyableOrMovable {
  NonCopyableOrMovable() = default;
  NonCopyableOrMovable(const NonCopyableOrMovable&) = delete;
  NonCopyableOrMovable(NonCopyableOrMovable&&) = delete;
  NonCopyableOrMovable& operator=(const NonCopyableOrMovable&) = delete;
  NonCopyableOrMovable& operator=(NonCopyableOrMovable&&) = delete;
};

template <typename Tp>
struct BadConstructors {
  BadConstructors(BadConstructors&&) {}
  BadConstructors(const BadConstructors&) {}
};

class Base {
public:
  virtual ~Base() {}
};

TEST(TypeTraitsTest, TestTrivialityBadConstructors) {
  using BadType = BadConstructors<int>;

  EXPECT_FALSE(polly::is_trivially_move_constructible<BadType>::value);
  EXPECT_FALSE(polly::is_trivially_copy_constructible<BadType>::value);
}

TEST(TypeTraitsTest, TestTrivialMoveCtor) {
  // Verify that arithmetic types and pointers have trivial move
  // constructors.
  EXPECT_TRUE(polly::is_trivially_move_constructible<bool>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<char>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<unsigned char>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<signed char>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<wchar_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<int>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<unsigned int>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<int16_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<uint16_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<int64_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<uint64_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<float>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<double>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<long double>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<const std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<const Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<std::string**>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<Trivial**>::value);

  // Reference types
  EXPECT_TRUE(polly::is_trivially_move_constructible<int&>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<int&&>::value);

  // types with compiler generated move ctors
  EXPECT_TRUE(polly::is_trivially_move_constructible<Trivial>::value);
  EXPECT_TRUE(polly::is_trivially_move_constructible<TrivialMoveCtor>::value);

  // Verify that types without them (i.e. nontrivial or deleted) are not.
  EXPECT_FALSE(polly::is_trivially_move_constructible<NontrivialCopyCtor>::value);
  EXPECT_FALSE(polly::is_trivially_move_constructible<DeletedCopyCtor>::value);
  EXPECT_FALSE(polly::is_trivially_move_constructible<NonCopyableOrMovable>::value);

  // type with nontrivial destructor are nontrivial move construbtible
  EXPECT_FALSE(polly::is_trivially_move_constructible<NontrivialDestructor>::value);

  // types with vtables
  EXPECT_FALSE(polly::is_trivially_move_constructible<Base>::value);

  // Verify that simple_pair of such types is trivially move constructible
  EXPECT_TRUE((polly::is_trivially_move_constructible<Pair<int, char*>>::value));
  EXPECT_TRUE((polly::is_trivially_move_constructible<Pair<int, Trivial>>::value));
  EXPECT_TRUE((polly::is_trivially_move_constructible<Pair<int, TrivialMoveCtor>>::value));

  // Verify that types without trivial move constructors are
  // correctly marked as such.
  EXPECT_FALSE(polly::is_trivially_move_constructible<std::string>::value);
  EXPECT_FALSE(polly::is_trivially_move_constructible<std::vector<int>>::value);

  // Verify that simple_pairs of types without trivial move constructors
  // are not marked as trivial.
  EXPECT_FALSE((polly::is_trivially_move_constructible<Pair<int, std::string>>::value));
  EXPECT_FALSE((polly::is_trivially_move_constructible<Pair<std::string, int>>::value));

  // Verify that arrays are not
  using int10 = int[10];
  EXPECT_FALSE(polly::is_trivially_move_constructible<int10>::value);
}

TEST(TypeTraitsTest, TestTrivialCopyCtor) {
  // Verify that arithmetic types and pointers have trivial copy
  // constructors.
  EXPECT_TRUE(polly::is_trivially_copy_constructible<bool>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<char>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<unsigned char>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<signed char>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<wchar_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<int>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<unsigned int>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<int16_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<uint16_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<int64_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<uint64_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<float>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<double>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<long double>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<const std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<const Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<std::string**>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<Trivial**>::value);

  // Reference types
  EXPECT_TRUE(polly::is_trivially_copy_constructible<int&>::value);
  EXPECT_FALSE(polly::is_trivially_copy_constructible<int&&>::value);

  // types with compiler generated copy ctors
  EXPECT_TRUE(polly::is_trivially_copy_constructible<Trivial>::value);
  EXPECT_TRUE(polly::is_trivially_copy_constructible<TrivialCopyCtor>::value);

  // Verify that types without them (i.e. nontrivial or deleted) are not.
  EXPECT_FALSE(polly::is_trivially_copy_constructible<NontrivialCopyCtor>::value);
  EXPECT_FALSE(polly::is_trivially_copy_constructible<DeletedCopyCtor>::value);
  EXPECT_FALSE(polly::is_trivially_copy_constructible<MovableNonCopyable>::value);
  EXPECT_FALSE(polly::is_trivially_copy_constructible<NonCopyableOrMovable>::value);

  // type with nontrivial destructor are nontrivial copy construbtible
  EXPECT_FALSE(polly::is_trivially_copy_constructible<NontrivialDestructor>::value);

  // types with vtables
  EXPECT_FALSE(polly::is_trivially_copy_constructible<Base>::value);

  // Verify that simple_pair of such types is trivially copy constructible
  EXPECT_TRUE((polly::is_trivially_copy_constructible<Pair<int, char*>>::value));
  EXPECT_TRUE((polly::is_trivially_copy_constructible<Pair<int, Trivial>>::value));
  EXPECT_TRUE((polly::is_trivially_copy_constructible<Pair<int, TrivialCopyCtor>>::value));

  // Verify that types without trivial copy constructors are
  // correctly marked as such.
  EXPECT_FALSE(polly::is_trivially_copy_constructible<std::string>::value);
  EXPECT_FALSE(polly::is_trivially_copy_constructible<std::vector<int>>::value);

  // Verify that simple_pairs of types without trivial copy constructors
  // are not marked as trivial.
  EXPECT_FALSE((polly::is_trivially_copy_constructible<Pair<int, std::string>>::value));
  EXPECT_FALSE((polly::is_trivially_copy_constructible<Pair<std::string, int>>::value));

  // Verify that arrays are not
  using int10 = int[10];
  EXPECT_FALSE(polly::is_trivially_copy_constructible<int10>::value);
}

TEST(TypeTraitsTest, TestTrivialMoveAssign) {
  // Verify that arithmetic types and pointers have trivial move
  // assignment operators.
  EXPECT_TRUE(polly::is_trivially_move_assignable<bool>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<char>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<unsigned char>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<signed char>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<wchar_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<int>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<unsigned int>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<int16_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<uint16_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<int64_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<uint64_t>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<float>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<double>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<long double>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<const std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<const Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<std::string**>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<Trivial**>::value);

  // const qualified types are not assignable
  EXPECT_FALSE(polly::is_trivially_move_assignable<const int>::value);

  // types with compiler generated move assignment
  EXPECT_TRUE(polly::is_trivially_move_assignable<Trivial>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<TrivialMoveAssign>::value);

  // Verify that types without them (i.e. nontrivial or deleted) are not.
  EXPECT_FALSE(polly::is_trivially_move_assignable<NontrivialCopyAssign>::value);
  EXPECT_FALSE(polly::is_trivially_move_assignable<DeletedCopyAssign>::value);
  EXPECT_FALSE(polly::is_trivially_move_assignable<NonCopyableOrMovable>::value);

  // types with vtables
  EXPECT_FALSE(polly::is_trivially_move_assignable<Base>::value);

  // Verify that simple_pair is trivially assignable
  EXPECT_TRUE((polly::is_trivially_move_assignable<Pair<int, char*>>::value));
  EXPECT_TRUE((polly::is_trivially_move_assignable<Pair<int, Trivial>>::value));
  EXPECT_TRUE((polly::is_trivially_move_assignable<Pair<int, TrivialMoveAssign>>::value));

  // Verify that types not trivially move assignable are
  // correctly marked as such.
  EXPECT_FALSE(polly::is_trivially_move_assignable<std::string>::value);
  EXPECT_FALSE(polly::is_trivially_move_assignable<std::vector<int>>::value);

  // Verify that simple_pairs of types not trivially move assignable
  // are not marked as trivial.
  EXPECT_FALSE((polly::is_trivially_move_assignable<Pair<int, std::string>>::value));
  EXPECT_FALSE((polly::is_trivially_move_assignable<Pair<std::string, int>>::value));

  // Verify that arrays are not trivially move assignable
  using int10 = int[10];
  EXPECT_FALSE(polly::is_trivially_move_assignable<int10>::value);

  // Verify that references are handled correctly
  EXPECT_TRUE(polly::is_trivially_move_assignable<Trivial&&>::value);
  EXPECT_TRUE(polly::is_trivially_move_assignable<Trivial&>::value);
}

TEST(TypeTraitsTest, TestTrivialCopyAssign) {
  // Verify that arithmetic types and pointers have trivial copy
  // assignment operators.
  EXPECT_TRUE(polly::is_trivially_copy_assignable<bool>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<char>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<unsigned char>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<signed char>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<wchar_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<int>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<unsigned int>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<int16_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<uint16_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<int64_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<uint64_t>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<float>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<double>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<long double>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<const std::string*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<const Trivial*>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<std::string**>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<Trivial**>::value);

  // const qualified types are not assignable
  EXPECT_FALSE(polly::is_trivially_copy_assignable<const int>::value);

  // types with compiler generated copy assignment
  EXPECT_TRUE(polly::is_trivially_copy_assignable<Trivial>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<TrivialCopyAssign>::value);

  // Verify that types without them (i.e. nontrivial or deleted) are not.
  EXPECT_FALSE(polly::is_trivially_copy_assignable<NontrivialCopyAssign>::value);
  EXPECT_FALSE(polly::is_trivially_copy_assignable<DeletedCopyAssign>::value);
  EXPECT_FALSE(polly::is_trivially_copy_assignable<MovableNonCopyable>::value);
  EXPECT_FALSE(polly::is_trivially_copy_assignable<NonCopyableOrMovable>::value);

  // types with vtables
  EXPECT_FALSE(polly::is_trivially_copy_assignable<Base>::value);

  // Verify that simple_pair is trivially assignable
  EXPECT_TRUE((polly::is_trivially_copy_assignable<Pair<int, char*>>::value));
  EXPECT_TRUE((polly::is_trivially_copy_assignable<Pair<int, Trivial>>::value));
  EXPECT_TRUE((polly::is_trivially_copy_assignable<Pair<int, TrivialCopyAssign>>::value));

  // Verify that types not trivially copy assignable are
  // correctly marked as such.
  EXPECT_FALSE(polly::is_trivially_copy_assignable<std::string>::value);
  EXPECT_FALSE(polly::is_trivially_copy_assignable<std::vector<int>>::value);

  // Verify that simple_pairs of types not trivially copy assignable
  // are not marked as trivial.
  EXPECT_FALSE((polly::is_trivially_copy_assignable<Pair<int, std::string>>::value));
  EXPECT_FALSE((polly::is_trivially_copy_assignable<Pair<std::string, int>>::value));

  // Verify that arrays are not trivially copy assignable
  using int10 = int[10];
  EXPECT_FALSE(polly::is_trivially_copy_assignable<int10>::value);

  // Verify that references are handled correctly
  EXPECT_TRUE(polly::is_trivially_copy_assignable<Trivial&&>::value);
  EXPECT_TRUE(polly::is_trivially_copy_assignable<Trivial&>::value);
}

namespace internal {
struct DeletedSwap {};

void swap(DeletedSwap&, DeletedSwap&) = delete;

struct SpecialNoexceptSwap {
  SpecialNoexceptSwap(SpecialNoexceptSwap&&) {}
  SpecialNoexceptSwap& operator=(SpecialNoexceptSwap&&) { return *this; }
  ~SpecialNoexceptSwap() = default;
};

void swap(SpecialNoexceptSwap&, SpecialNoexceptSwap&) noexcept {}

}  // namespace adl_namespace

TEST(TypeTraitsTest, IsSwappable) {
  EXPECT_TRUE(polly::is_swappable<int>::value);

  struct S {};
  EXPECT_TRUE(polly::is_swappable<S>::value);

  struct NoConstructor {
    NoConstructor(NoConstructor&&) = delete;
    NoConstructor& operator=(NoConstructor&&) { return *this; }
    ~NoConstructor() = default;
  };
  EXPECT_EQ(polly::is_swappable<NoConstructor>::value, polly::StdSwapIsUnconstrained::value);

  struct NoAssign {
    NoAssign(NoAssign&&) {}
    NoAssign& operator=(NoAssign&&) = delete;
    ~NoAssign() = default;
  };
  EXPECT_EQ(polly::is_swappable<NoAssign>::value, polly::StdSwapIsUnconstrained::value);

  EXPECT_FALSE(polly::is_swappable<internal::DeletedSwap>::value);
  EXPECT_TRUE(polly::is_swappable<internal::SpecialNoexceptSwap>::value);
}

TEST(TypeTraitsTest, IsNothrowSwappable) {
  EXPECT_TRUE(polly::is_nothrow_swappable<int>::value);

  struct NonNoexceptMoves {
    NonNoexceptMoves(NonNoexceptMoves&&) {}
    NonNoexceptMoves& operator=(NonNoexceptMoves&&) { return *this; }
    ~NonNoexceptMoves() = default;
  };
  EXPECT_FALSE(polly::is_nothrow_swappable<NonNoexceptMoves>::value);

  struct NoConstructor {
    NoConstructor(NoConstructor&&) = delete;
    NoConstructor& operator=(NoConstructor&&) { return *this; }
    ~NoConstructor() = default;
  };
  EXPECT_FALSE(polly::is_nothrow_swappable<NoConstructor>::value);

  struct NoAssign {
    NoAssign(NoAssign&&) {}
    NoAssign& operator=(NoAssign&&) = delete;
    ~NoAssign() = default;
  };
  EXPECT_FALSE(polly::is_nothrow_swappable<NoAssign>::value);

  EXPECT_FALSE(polly::is_nothrow_swappable<internal::DeletedSwap>::value);
  EXPECT_TRUE(polly::is_nothrow_swappable<internal::SpecialNoexceptSwap>::value);
}

}  // namespace
