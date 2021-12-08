#pragma once

#include <type_traits>

#include "stubs/config.h"
#include "stubs/identity.h"

namespace polly {
template <typename...>
struct Or;

template <>
struct Or<>: public std::false_type {};

template <typename T1>
struct Or<T1>: public T1 {};

template <typename T1, typename T2>
struct Or<T1, T2>: public std::conditional<T1::value, T1, T2>::type {};

template <typename T1, typename T2, typename T3, typename... Tn>
struct Or<T1, T2, T3, Tn...>
    : public std::conditional<T1::value, T1, Or<T2, T3, Tn...>>::type {};

template <typename...>
struct And;

template <>
struct And<>: public std::true_type {};

template <typename T1>
struct And<T1>: public T1 {};

template <typename T1, typename T2>
struct And<T1, T2>: public std::conditional<T1::value, T2, T1>::type {};

template <typename T1, typename T2, typename T3, typename... Tn>
struct And<T1, T2, T3, Tn...>
    : public std::conditional<T1::value, And<T2, T3, Tn...>, T1>::type {};

template <typename Tp>
struct Not: public std::integral_constant<bool, !Tp::value> {};

template <typename... Tn>
using Requires = typename enable_if<And<Tn...>::value, bool>::type;

template<typename Tp>
using remove_cvref =
    std::remove_cv<typename std::remove_reference<Tp>::type>;

template<typename Tp>
using remove_cvref_t = typename remove_cvref<Tp>::type;

namespace type_traits_internal {
template <typename Tp>
union SingleMemberUnion {
  Tp val;
};

template <typename Tp>
struct is_trivially_copy_constructible_object
    : std::integral_constant<
        bool,
        std::is_copy_constructible<SingleMemberUnion<Tp>>::value &&
        std::is_trivially_destructible<Tp>::value> {};

template <typename Tp>
struct is_trivially_move_constructible_object
    : std::integral_constant<
        bool,
        std::is_move_constructible<SingleMemberUnion<Tp>>::value &&
        std::is_trivially_destructible<Tp>::value> {};

} // namespace type_traits_internal

// is_trivially_copy_constructible
template <typename Tp>
struct is_trivially_copy_constructible
    : std::conditional<
      std::is_object<Tp>::value && !std::is_array<Tp>::value,
      typename type_traits_internal::is_trivially_copy_constructible_object<Tp>::type,
      typename std::is_lvalue_reference<Tp>::type>::type {
#if defined(POLLY_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE)
 private:
  static constexpr bool compliant =
      std::is_trivially_copy_constructible<Tp>::value ==
      is_trivially_copy_constructible::value;
  static_assert(compliant || std::is_trivially_copy_constructible<Tp>::value,
                "Not compliant with std::is_trivially_copy_constructible; "
                "Standard: false, Implementation: true");
  static_assert(compliant || !std::is_trivially_copy_constructible<Tp>::value,
                "Not compliant with std::is_trivially_copy_constructible; "
                "Standard: true, Implementation: false");
#endif // POLLY_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE
};

// is_trivially_move_constructible
template <typename Tp>
struct is_trivially_move_constructible
    : std::conditional<
      std::is_object<Tp>::value && !std::is_array<Tp>::value,
      typename type_traits_internal::is_trivially_move_constructible_object<Tp>::type,
      typename std::is_lvalue_reference<Tp>::type
    >::type {
#if defined(POLLY_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE)
 private:
  static constexpr bool compliant =
      std::is_trivially_move_constructible<Tp>::value ==
      is_trivially_move_constructible::value;
  static_assert(compliant || std::is_trivially_move_constructible<Tp>::value,
                "Not compliant with std::is_trivially_copy_constructible; "
                "Standard: false, Implementation: true");
  static_assert(compliant || !std::is_trivially_move_constructible<Tp>::value,
                "Not compliant with std::is_trivially_copy_constructible; "
                "Standard: true, Implementation: false");
#endif // POLLY_HAVE_STD_IS_TRIVIALLY_CONSTRUCTIBLE
};

template <typename>
struct is_trivially_move_assignable;

namespace type_traits_internal {
template <typename Tp>
struct is_trivially_move_assignable_reference: std::false_type {};

template <typename Tp>
struct is_trivially_move_assignable_reference<Tp&>
    : is_trivially_move_assignable<Tp>::type {};

template <typename Tp>
struct is_trivially_move_assignable_reference<Tp&&>
    : is_trivially_move_assignable<Tp>::type {};
} // namespace type_traits_internal

// is_trivially_copy_assignable
template <typename Tp>
struct is_trivially_copy_assignable
    : std::integral_constant<
        bool,
        __has_trivial_assign(typename std::remove_reference<Tp>::type) &&
        std::is_copy_assignable<Tp>::value> {
#if defined(POLLY_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE)
 private:
  static constexpr bool compliant =
      std::is_trivially_copy_assignable<Tp>::value ==
      is_trivially_copy_assignable::value;
  static_assert(compliant || std::is_trivially_copy_assignable<Tp>::value,
                "Not compliant with std::is_trivially_copy_assignable; "
                "Standard: false, Implementation: true");
  static_assert(compliant || !std::is_trivially_copy_assignable<Tp>::value,
                "Not compliant with std::is_trivially_copy_assignable; "
                "Standard: true, Implementation: false");
#endif  // POLLY_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE
};

// is_trivially_move_assignable
template <typename Tp>
struct is_trivially_move_assignable
    : std::conditional<
        std::is_object<Tp>::value &&
        !std::is_array<Tp>::value &&
        std::is_move_assignable<Tp>::value,
        typename std::is_move_assignable<type_traits_internal::SingleMemberUnion<Tp>>::type,
        typename type_traits_internal::is_trivially_move_assignable_reference<Tp>::type>::type {
#if defined(POLLY_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE)
 private:
  static constexpr bool compliant =
      std::is_trivially_move_assignable<Tp>::value ==
      is_trivially_move_assignable::value;
  static_assert(compliant || std::is_trivially_move_assignable<Tp>::value,
                "Not compliant with std::is_trivially_move_assignable; "
                "Standard: false, Implementation: true");
  static_assert(compliant || !std::is_trivially_move_assignable<Tp>::value,
                "Not compliant with std::is_trivially_move_assignable; "
                "Standard: true, Implementation: false");
#endif  // POLLY_HAVE_STD_IS_TRIVIALLY_ASSIGNABLE
};




} // namespace polly