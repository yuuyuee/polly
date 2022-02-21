#pragma once

#include "stubs/internal/config.h"

#include "stubs/type_traits.h"
#include "stubs/utility.h"

namespace polly {
template <typename...>
class variant;

// Helper objects
POLLY_INLINE_CONSTEXPR(size_t, variant_npos, static_cast<size_t>(-1));

namespace variant_internal {
template <std::size_t N, typename... Types>
struct nth_type;

template <std::size_t N, typename First, typename... Last>
struct nth_type<N, First, Last...>: public nth_type<N - 1, Last...> {};

template <typename First, typename... Last>
struct nth_type<0, First, Last...> {
  using type = First;
};

template <typename... Types>
struct Traits {
  struct is_default_ctor {
    static constexpr bool value =
        std::is_default_constructible<typename nth_type<0, Types...>::type>::value;
  };

  struct is_copy_ctor {
    static constexpr bool value =
        conjunction<std::is_copy_constructible<Types>...>::value;
  };

  struct is_move_ctor {
    static constexpr bool value =
        conjunction<std::is_move_constructible<Types>...>::value;
  };

  struct is_copy_assign {
    static constexpr bool value =
        Traits<Types...>::is_copy_ctor::value &&
        conjunction<std::is_copy_assignable<Types>...>::value;
  };

  struct is_move_assign {
    static constexpr bool value =
        Traits<Types...>::is_move_ctor::value &&
        conjunction<std::is_move_assignable<Types>...>::value;
  };

  struct is_trivial_dtor {
    static constexpr bool value =
        conjunction<std::is_trivially_destructible<Types>...>::value;
  };

  struct is_trivial_copy_ctor {
    static constexpr bool value =
        conjunction<is_trivially_copy_constructible<Types>...>::value;
  };

  struct is_trivial_move_ctor {
    static constexpr bool value =
        conjunction<is_trivially_move_constructible<Types>...>::value;
  };

  struct is_trivial_copy_assign {
    static constexpr bool value =
        Traits<Types...>::is_trivial_dtor::value &&
        Traits<Types...>::is_trivial_copy_ctor::value &&
        conjunction<is_trivially_copy_assignable<Types>...>::value;
  };

  struct is_trivial_move_assign {
    static constexpr bool value =
        Traits<Types...>::is_trivial_dtor::value &&
        Traits<Types...>::is_trivial_move_ctor::value &&
        conjunction<is_trivially_move_assignable<Types>...>::value;
  };

  struct is_nothrow_default_ctor {
    static constexpr bool value =
        std::is_nothrow_default_constructible<
            typename nth_type<0, Types...>::type
        >::value;
  };

  struct is_nothrow_copy_ctor {
    static constexpr bool value = false;
  };

  struct is_nothrow_move_ctor {
    static constexpr bool value =
        conjunction<std::is_nothrow_move_constructible<Types>...>::value;
  };

  struct is_nothrow_copy_assign {
    static constexpr bool value = false;
  };

  struct is_nothrow_move_assign {
    static constexpr bool value =
        Traits<Types...>::is_nothrow_move_ctor::value &&
        conjunction<is_trivially_move_assignable<Types>...>::value;
  };
};




template <typename... Types>
union Union {};

template <typename First, typename... Last>
union Union<First, Last...> {
  using union_type = Union<Last...>;

  constexpr Union(): last() {}

  template<typename... Args>
  constexpr explicit Union(in_place_index_t<0>, Args&&... args)
      : first(std::forward<Args>(args)...) {}

  template<std::size_t I, typename... Args>
  constexpr explicit Union(in_place_index_t<I>, Args&&... args)
      : last(in_place_index_t<I - 1>, std::forward<Args>(args)...) {}

  First first;
  union_type last;
};

template <typename... Types>
union UnionDestructible {};

template <typename First, typename... Last>
union UnionDestructible<First, Last...> {
  using union_type = UnionDestructible<Last...>;

  constexpr Union(): last() {}

  template<typename... Args>
  constexpr explicit Union(in_place_index_t<0>, Args&&... args)
      : first(std::forward<Args>(args)...) {}

  template<std::size_t I, typename... Args>
  constexpr explicit Union(in_place_index_t<I>, Args&&... args)
      : last(in_place_index_t<I - 1>, std::forward<Args>(args)...) {}

  ~UnionDestructible() {}

  First first;
  union_type last;
};

template <typename... Types>
using VariantUnion =
    std::conditional<
        std::is_trivially_destructible<Union<Types...>::value,
        Union<Types...>, UnionDestructible<Types...>>;

template <bool /* trivial dtor */, typename... Types>
struct VariantStorage;

template <typename... Types>
struct VariantStorage<false, Types...> {
  constexpr VariantStorage(): index(variant_npos) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantStorage(in_place_index_t<I>, Args&&... args)
      : state(in_place_index_t<I>, std::forward<Args>(args)...), index(I) {}

  ~VariantStorage() {}

  VariantUnion<Types...> state;
  std::size_t index;
};

template <typename... Types>
struct VariantStorage<true, Types...> {
  constexpr VariantStorage(): index(variant_npos) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantStorage(in_place_index_t<I>, Args&&... args)
      : state(in_place_index_t<I>, std::forward<Args>(args)...), index(I) {}

  VariantUnion<Types...> state;
  std::size_t index;
};

template <typename... Types>
using VariantStorageType =
    VariantStorage<Traits<Types...>::is_trivial_dtor::value, Types...>;

template <bool /* trivial copy ctor */, typename... Types>
struct CopyConstructorBase: public VariantStorageType<Types...> {
  using Base = VariantStorageType<Types...>;
  using Base::Base;

  CopyConstructorBase(const CopyConstructorBase& other)
      noexcept(Traits<Types...>::is_nothrow_copy_ctor::value) {
    // TODO:
  }
  CopyConstructorBase(CopyConstructorBase&&) = default;
  CopyConstructorBase& operator=(const CopyConstructorBase&) = default;
  CopyConstructorBase& operator=(CopyConstructorBase&&) = default;
};

template <typename... Types>
struct CopyConstructorBase<true, Types...>: public  VariantStorageType<Types...> {
  using Base = VariantStorageType<Types...>;
  using Base::Base;
};

template <typename... Types>
using CopyConstructorBaseType =
    CopyConstructorBase<Traits<Types...>::is_trivial_copy_ctor::value, Types...>;

template <bool /* trivial move ctor */, typename... Types>
struct MoveConstructorBase: public CopyConstructorBaseType<Types...> {
  using Base = CopyConstructorBaseType<Types...>;
  using Base::Base;

  MoveConstructorBase(MoveConstructorBase&& other)
      noexcept(Traits<Types...>::is_nothrow_move_ctor::value) {
    // TODO:
  }

  // TODO: desttructive_move, destructive_copy

  MoveConstructorBase(const MoveConstructorBase&) = default;
  MoveConstructorBase& operator=(const MoveConstructorBase&) = default;
  MoveConstructorBase& operator=(MoveConstructorBase&&) = default;
};

template <typename... Types>
struct MoveConstructorBase<true, Types...>: public CopyConstructorBaseType<Types...> {
  using Base = CopyConstructorBaseType<Types...>;
  using Base::Base;

  // TODO: desttructive_move, destructive_copy
};

template <typename... Types>
using MoveConstructorBaseType =
    MoveConstructorBase<Traits<Types...>::is_trivial_move_ctor::value, Types...>;

template <bool /* trivial copy assign */, typename... Types>
struct CopyAssignBase: public MoveConstructorBaseType<Types...> {
  using Base = MoveConstructorBaseType<Types...>;
  using Base::Base;

  CopyAssignBase& operator=(const CopyAssignBase& other)
      noexcept(Traits<Types...>::is_nothrow_copy_assign::value) {
    // TODO:
  }

  CopyAssignBase(const CopyAssignBase&) = default;
  CopyAssignBase(CopyAssignBase&&) = default;
  CopyAssignBase& operator=(CopyAssignBase&&) = default;
};

template <typename... Types>
struct CopyAssignBase<true, Types...>: public MoveConstructorBaseType<Types...> {
  using Base = MoveConstructorBaseType<Types...>;
  using Base::Base;
};

template <typename... Types>
using CopyAssignBaseType =
    CopyAssignBase<Traits<Types...>::is_trivial_copy_assign::value, Types...>;

template <bool /* trivial move assign */, typename... Types>
struct MoveAssignBase: public CopyAssignBaseType<Types...> {
  using Base = CopyAssignBaseType<Types...>;
  using Base::Base;

  MoveAssignBase& operator=(MoveAssignBase&& other)
      noexcept(Traits<Types...>::is_nothrow_move_assign::value) {
    // TODO
  }

  MoveAssignBase(const MoveAssignBase&) = default;
  MoveAssignBase(MoveAssignBase&&) = default;
  MoveAssignBase& operator=(const MoveAssignBase&) = default;
};

template <typename... Types>
struct MoveAssignBase: public CopyAssignBaseType<Types...> {
  using Base = CopyAssignBaseType<Types...>;
  using Base::Base;
};

template <typename... Types>
using MoveAssignBaseType =
    MoveAssignBase<Traits<Types...>::is_trivial_move_assign::value, Types...>;

template <typename... Types>
struct VariantBase: public MoveAssignBaseType<Tyeps...> {
  using Base = MoveAssignBaseType<Tyeps...>;

  constexpr VariantBase()
      noexcept(Traits<Types...>::is_nothrow_default_ctor)
      : VariantBase(in_place_index<0>) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantBase(in_place_index_t<I> i, Args&&... args)
      : Base(i, std::forward<Args>(args)...) {}

  VariantBase(const VariantBase&) = default;
  VariantBase(VariantBase&&) = default;
  VariantBase& operator=(const VariantBase&) = default;
  VariantBase& operator=(VariantBase&&) = default;
};
} // namespace variant_internal

} // namespace polly