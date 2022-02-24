#pragma once

#include "stubs/internal/config.h"

#include "stubs/type_traits.h"
#include "stubs/utility.h"
#include "stubs/internal/enable_special_members.h"

namespace polly {
template <typename...>
class variant;

// Helper objects
POLLY_INLINE_CONSTEXPR(size_t, variant_npos, static_cast<size_t>(-1));

// variant_size
// Provides access to the number of alternatives in a possibly cv-qualified
// variant as a compile-time constant expression.
template <typename>
struct variant_size;

template <typename... Types>
struct variant_size<variant<Types...>>
    : public std::integral_constant<std::size_t, sizeof...(Types)> {};

// Specialization for const qualified variant.
template <typename Variant>
struct variant_size<const Variant>
    : public variant_size<Variant>::value {};

// Specialization for volatile qualified variant.
template <typename Variant>
struct variant_size<volatile Variant>
    : public variant_size<Variant>::value {};

// Specialization for const volatile qualified variant.
template <typename Variant>
struct variant_size<const volatile Variant>
    : public variant_size<Variant>::value {};

namespace variant_internal {
template <typename Tp, typename Variant>
struct TypeCount;

template <typename Tp, typename Last>
struct TypeCount<Tp, variant<Last>>
    : public std::integral_constant<std::size_t, std::is_same<Tp, Last>::value> {};

template <typename Tp, typename First, typename... Last>
struct TypeCount<Tp, variant<First, Last...>>
    : public std::integral_constant<
        std::size_t,
        TypeCount<Tp, variant<First>>::value + TypeCount<Tp, variant<Last...>>::value> {};

template <typename Tp, typename Variant>
struct IndexOf: public std::integral_constan<std::size_t, 0> {};

template <typename Tp, typename First, typename... Last>
struct IndexOf<Tp, variant<Tp, First, Last...>>
    : public std::integral_constant<
        std::size_t,
        TypeCount<Tp, variant<First>>::value ? 0 : IndexOf<Tp, variant<Last...>>::value + 1
      > {};

template <std::size_t N, typename Variant>
struct TypeOf;

template <std::size_t N, typename First, typename... Last>
struct TypeOf<N, variant<First, Last...>>
    : public TypeOf<N - 1, variant<Last...>> {};

template <typename First, typename... Last>
struct TypeOf<0, variant<First, Last...>> {
  using type = First;
};

template <typename... Types>
struct Traits {
  struct is_default_ctor {
    static constexpr bool value =
        std::is_default_constructible<
            typename TypeOf<0, variant<Types...>>::type>::value;
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
            typename TypeOf<0, variant<Types...>>::type
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

template <std::size_t I>
using SizeType = std::integral_constant<std::size_t, I>;

template <typename Ti>
struct Array { Ti x[1]; };

template <typename Ti, typename Tp>
using ConvertCtorType = decltype(Array<Ti>{{std::declval<Tp>()}});

template <
    std::size_t I,
    typename Tp,
    typename Ti,
    bool Ti_is_cv_bool = std::is_same<typename std::remove_cv<Ti>::type, bool>,
    typename = void>
struct BuildFun {
  // This function means `using BuildFun<I, Tp, Ti>::Fn` is valid,
  // but only static functions will be considered in the call below.
  void Fn();
}

// For which Ti x[] = {std::forward<Tp>(t)}
template <std::size_t I, typename Tp, typename Ti>
struct BuildFun<I, Tp, Ti, false,
    void_t<ConvertCtorType<Ti, Tp>> {
  // Fn is function for type Ti with index I
  static SizeType<I> Fn(Ti);
};

// For which Ti is cv bool, remove_cvref_t<Tp> is bool
template <std::size_t I, typename Tp, typename Ti>
struct BuildFun<I, Tp, Ti, true,
    enable_if_t<std::is_same<remove_cvref_t<Tp>, bool>::value> {
  static SizeType<I> Fn(Ti);
};

tempate <typename Tp, typename Variant,
    typename = make_index_sequence<variant_size<Variant>::value>>
struct BuildFuns;

template <typename Tp, typename... Ti, std::size_t... I>
struct BuildFuns<Tp, variant<Ti...>, index_sequence<I...>>
    public BuildFun<I, Tp, Ti>... {
  using BuildFun<I, Tp, Ti>::Fn;
};

tmeplate <typename Tp, typename Variant>
using FunType = decltype(BuildFuns<Tp, Variant>::Fn(std::declval<Tp>()));

template <typename Tp, typename Variant, typename = void>
struct IndexOfConstructType: public SizeType<variant_npos> {};

template <typename Tp, typename Variant, typename = void_t<FunType<Tp, Variant>>>
struct IndexOfCtorType: public FunType<Tp, Variant> {};


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
        std::is_trivially_destructible<Union<Types...>>::value,
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
      noexcept(Traits<Types...>::is_nothrow_default_ctor::value)
      : VariantBase(in_place_index<0>) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantBase(in_place_index_t<I> i, Args&&... args)
      : Base(i, std::forward<Args>(args)...) {}

  VariantBase(const VariantBase&) = default;
  VariantBase(VariantBase&&) = default;
  VariantBase& operator=(const VariantBase&) = default;
  VariantBase& operator=(VariantBase&&) = default;
};

template <typename... Types>
using VariantEnableDefaultConstructor =
    polly::enable_default_constructor<
        variant_internal::Traits<Types...>::is_default_ctor::value
        variant<Types...>>;

template <typename... Types>
using VariantEnableCopyMove =
    polly::enable_copy_move<
        variant_internal::Traits<Types...>::is_copy_ctor::value,
        variant_internal::Traits<Types...>::is_copy_assign::value,
        variant_internal::Traits<Types...>::is_move_ctor::value,
        variant_internal::Traits<Types...>::is_move_assign::value,
        variant<Types...>>;

template <typename Tp>
struct NotInPlaceTagImpl: public std::true_type {};

template <typename Tp>
struct NotInPlaceTagImpl<in_place_type_t<Tp>>: public std::false_type {};

template <std::size_t I>
struct NotInPlaceTagImpl<in_place_index_t<I>>: public std::false_type {};

template <typename Tp>
using NotInPlaceTag = NotInPlaceTagImpl<remove_cvref_t<Tp>>;

template <typename Tp, typename Variant>
using NotSelf = negation<std::is_same<Variant, remove_cvref_t<Tp>>>;

} // namespace variant_internal

} // namespace polly