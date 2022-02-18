#pragma once

#include "stubs/internal/config.h"

#include "stubs/type_traits.h"
#include "stubs/utility.h"

namespace polly {
template <typename...>
class variant;

namespace variant_internal {
struct enable_default_constructors_tag {
  explicit constexpr enable_default_constructors_tag() = default;
};

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
        std::is_default_constructible<
            typename nth_type<0, Types...>::type
        >::value;
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



template <typename Tp, bool = std::is_trivially_destructible<Tp>::value>
struct Uninitialized {
  template <typename... Args>
  constexpr Uninitialized(in_place_index_t<0>, Args&&... args) {
    ::new (reinterpret_cast<void*>(std::addressof(value)))
        Tp(std::forward<Args>(args)...);
  }

  Tp& get() & noexcept {
    return value;
  }

  const Tp& get() const& noexcept {
    return value;
  }

  Tp&& get() && noexcept {
    return std::move(value);
  }

  const Tp&& get() const&& noexcept {
    return std::move(value);
  }


  Tp value;
}

template <typename Tp>
struct Uninitialized<Tp, true> {

};



template <typename... Types>
union VariantUnion {};

template <typename First, typename... Last>
union VariantUnion<First, Last...> {
  constexpr VariantUnion()
};




template <typename... Types>
struct VariantBase: MoveAssignBase<Types...> {
  using Base = MoveAssignBase<Types...>;
};

} // namespace variant_internal


} // namespace polly