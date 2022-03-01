#pragma once

#include "stubs/internal/config.h"

#include "stubs/internal/enable_special_members.h"
#include "stubs/invoke.h"
#include "stubs/type_traits.h"
#include "stubs/utility.h"

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

// variant_alternative
// Provides compile-time indexed access to the types of the alternatives of the
// possibly cv-qualified variant, combining cv-qualifications of the variant
// with the cv-qualifications of the alternative.
template <std::size_t I, typename Variant>
struct variant_alternative;

template <std::size_t I, typename First, typename... Last>
struct variant_alternative<I, variant<First, Last...>>
    : public variant_alternative<I - 1, variant<Last...>> {};

template <typename First, typename... Last>
struct variant_alternative<0, variant<First, Last...>> {
  using type = First;
};

template <std::size_t I, typename Variant>
using variant_alternative_t = typename variant_alternative<I, Variant>::type;

// Specialization for const qualified variant.
template <std::size_t I, typename Tp>
struct variant_alternative<I, const Tp> {
  using type = const typename variant_alternative<I, Tp>::type;
};

// Specialization for volatile qualified variant.
template <std::size_t I, typename Tp>
struct variant_alternative<I, volatile Tp> {
  using type = volatile typename variant_alternative<I, Tp>::type;
};

// Specialization for const volatile qualified variant.
template <std::size_t I, typename Tp>
struct variant_alternative<I, const volatile Tp> {
  using type = const volatile typename variant_alternative<I, Tp>::type;
};

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
        std::is_same<Tp, First>::value ? 0 : IndexOf<Tp, variant<Last...>>::value + 1> {};

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
    bool Ti_is_cv_bool = std::is_same<remove_cv_t<Ti>, bool>::value,
    typename = void>
struct BuildFun {
  // This function means `using BuildFun<I, Tp, Ti>::Fn` is valid,
  // but only static functions will be considered in the call below.
  void Fn();
};

// For which Ti x[] = {std::forward<Tp>(t)}
template <std::size_t I, typename Tp, typename Ti>
struct BuildFun<I, Tp, Ti, false, void_t<ConvertCtorType<Ti, Tp>>> {
  // Fn is function for type Ti with index I
  static SizeType<I> Fn(Ti);
};

// For which Ti is cv bool, remove_cvref_t<Tp> is bool
template <std::size_t I, typename Tp, typename Ti>
struct BuildFun<I, Tp, Ti, true,
    enable_if_t<std::is_same<remove_cvref_t<Tp>, bool>::value>> {
  static SizeType<I> Fn(Ti);
};

template <
    typename Tp,
    typename Variant,
    typename = typename make_index_sequence<
        variant_size<Variant>::value>::type>
struct BuildFuns;

template <typename Tp, typename... Ti, std::size_t... I>
struct BuildFuns<Tp, variant<Ti...>, index_sequence<I...>>
    : BuildFun<I, Tp, Ti>... {
  using BuildFun<I, Tp, Ti>::Fn...;
};

template <typename Tp, typename Variant>
using FunType = decltype(BuildFuns<Tp, Variant>::Fn(std::declval<Tp>()));

template <typename Tp, typename Variant, typename = void>
struct IndexOfCtorType: public SizeType<variant_npos> {};

template <typename Tp, typename Variant,
    typename = void_t<FunType<Tp, Variant>>>
struct IndexOfCtorType: public FunType<Tp, Variant> {};

template <std::size_t I, typename Variant>
struct VariantAccessResultTypeImpl;

template <std::size_t I, template <typename...> class VariantTemplate, typename... Types>
struct VariantAccessResultTypeImpl<I, VariantTemplate<Types...>&> {
  using type = variant_alternative_t<I, variant<Types...>>&;
};

template <std::size_t I, template <typename...> class VariantTemplate, typename... Types>
struct VariantAccessResultTypeImpl<I, const VariantTemplate<Types...>&> {
  using type = const variant_alternative_t<I, variant<Types...>>&;
};

template <std::size_t I, template <typename...> class VariantTemplate, typename... Types>
struct VariantAccessResultTypeImpl<I, VariantTemplate<Types...>&&> {
  using type = variant_alternative_t<I, variant<Types...>>&&;
};

template <std::size_t I, template <typename...> class VariantTemplate, typename... Types>
struct VariantAccessResultTypeImpl<I, const VariantTemplate<Types...>&&> {
  using type = const variant_alternative_t<I, variant<Types...>>&&;
};

template <std::size_t I, typename Variant>
using VariantAccessResultType =
    typename VariantAccessResultTypeImpl<I, Variant&&>::type;

template <typename Visitor, typename... Variants>
using VisitResultType =
    polly::invoke_result_t<Visitor,
                           VariantAccessResultType<0, Variants>...>;

// class TrivialDestructibleObject
// Uninitialized<T> is guaranteed to be a trivially destructible type
// even if Tp is not.
template <typename Tp, bool = is_trivially_destructible<Tp>::value>
struct TrivialDestructibleObject;

template <typename Tp>
struct TrivialDestructibleObject<Tp, false> {
  template <typename... Args>
  TrivialDestructibleObject(in_place_index_t<0>, Args&&... args) {
    ::new (reinterpret_cast<void*>(std::addressof(&storage)))
        Tp(std::forward<Args>(args)...);
  }

  Tp& Get() noexcept {
    return *reinterpret_cast<Tp*>(std::addressof(&storage));
  }

  constexpr const Tp& Get() noexcept {
    return *reinterpret_cast<const Tp*>(std::addressof(&storage));
  }

  Tp&& Get() noexcept {
    return std::move(*reinterpret_cast<Tp*>(std::addressof(&storage)));
  }

  constexpr const Tp&& Get() noexcept {
    return std::move(*reinterpret_cast<const Tp*>(std::addressof(&storage)));
  }

  std::aligned_storage<sizeof(Tp)>::type storage;
};

template <typename Tp>
struct TrivialDestructibleObject<Tp, true> {
  template <typename... Args>
  TrivialDestructibleObject(polly::in_place_index_t<0>, Args&&... args)
      : storage(std::forward<Args>(args)...) {}

  Tp& Get() noexcept {
    return storage;
  }

  constexpr const Tp& Get() noexcept {
    return storage;
  }

  Tp&& Get() noexcept {
    return std::move(storage);
  }

  constexpr const Tp&& Get() noexcept {
    return std::move(storage);
  }

  Tp storage;
};

// class VariantUnion
template <typename... Types>
union VariantUnion {};

template <typename First, typename... Last>
union VariantUnion<First, Last...> {
  constexpr VariantUnion(): last() {}

  template <typename... Args>
  constexpr explicit VariantUnion(polly::in_place_index_t<0>, Args&&... args)
      : first(polly::in_place_index_t<0>, std::forward<Args>(args)...) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantUnion(polly::in_place_index_t<I>, Args&&... args)
      : last(polly::in_place_index_t<I - 1>, std::forward<Args>(args)...) {}

  TrivialDestructibleObject<First> first;
  VariantUnion<Last...> last;
};

template <typename Up>
VariantAccessResultType<0, Up>
Get(in_place_index_t<0>, Up&& u) noexcept {
  return std::forward<Up>(u).first.Get();
}

template <std::size_t I, typename Up>
VariantAccessResultType<I, Up>
Get(polly::in_place_index_t<I>, Up&& u) noexcept {
  return Get(polly::in_place_index_t<I - 1>, std::forward<Up>(u).last);
}

template <typename Tp>
void Destroy(const Tp* p) {
  p-~Tp();
}

template <bool /* trivial dtor */, typename... Types>
struct VariantStorageImpl;

template <typename... Types>
struct VariantStorageImpl<false, Types...> {
  constexpr VariantStorageImpl(): index(variant_npos) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantStorageImpl(polly::in_place_index_t<I>, Args&&... args)
      : state(polly::in_place_index_t<I>, std::forward<Args>(args)...), index(I) {}

  ~VariantStorageImpl() { Reset(); }

  constexpr bool Valid() const noexcept {
    return index != variant_npos;
  }

  void Reset() {
    if (Valid()) {
      Destroy(&Get(polly::in_place_index_t<index>, state));
      index = variant_npos;
    }
  }

  VariantUnion<Types...> state;
  std::size_t index;
};

template <typename... Types>
struct VariantStorageImpl<true, Types...> {
  constexpr VariantStorageImpl(): index(variant_npos) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantStorageImpl(polly::in_place_index_t<I>, Args&&... args)
      : state(polly::in_place_index_t<I>, std::forward<Args>(args)...), index(I) {}

  constexpr bool Valid() const noexcept {
    return index != variant_npos;
  }

  void Reset() {
    index = variant_npos;
  }

  VariantUnion<Types...> state;
  std::size_t index;
};

template <typename... Types>
using VariantStorage =
    VariantStorageImpl<Traits<Types...>::is_trivial_dtor::value, Types...>;

template <std::size_t I, typename Variant>
VariantAccessResultType<I, Variant>
Get(polly::in_place_index_t<I>, Variant&& v) noexcept {
  return Get(polly::in_place_index_t<I>, std::forward<Variant>(v).state);
}

// visit helper


template <bool /* trivial copy ctor */, typename... Types>
struct CopyConstructorBase: public VariantStorage<Types...> {
  using Base = VariantStorage<Types...>;
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
struct CopyConstructorBase<true, Types...>: public  VariantStorage<Types...> {
  using Base = VariantStorage<Types...>;
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
struct MoveConstructorBase<true, Types...>
    : public CopyConstructorBaseType<Types...> {
  using Base = CopyConstructorBaseType<Types...>;
  using Base::Base;

  // TODO: desttructive_move, destructive_copy
};

template <typename... Types>
using MoveConstructorBaseType =
    MoveConstructorBase<Traits<Types...>::is_trivial_move_ctor::value,
                        Types...>;

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
struct CopyAssignBase<true, Types...>
    : public MoveConstructorBaseType<Types...> {
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
      : VariantBase(polly::in_place_index<0>) {}

  template <std::size_t I, typename... Args>
  constexpr explicit VariantBase(polly::in_place_index_t<I> i, Args&&... args)
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
struct NotInPlaceTagImpl<polly::in_place_type_t<Tp>>: public std::false_type {};

template <std::size_t I>
struct NotInPlaceTagImpl<polly::in_place_index_t<I>>: public std::false_type {};

template <typename Tp>
using NotInPlaceTag = NotInPlaceTagImpl<polly::remove_cvref_t<Tp>>;

template <typename Tp, typename Variant>
using NotSelf = polly::negation<std::is_same<Variant, polly::remove_cvref_t<Tp>>>;



} // namespace variant_internal

} // namespace polly