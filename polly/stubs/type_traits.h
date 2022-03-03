#pragma once

#include "polly/stubs/internal/config.h"

#include <type_traits>
#include <utility>

namespace polly {
template <typename...>
struct disjunction;

template <>
struct disjunction<>: std::false_type {};

template <typename T1>
struct disjunction<T1>: T1 {};

template <typename T1, typename T2>
struct disjunction<T1, T2>: std::conditional<T1::value, T1, T2>::type {};

template <typename T1, typename T2, typename T3, typename... Tn>
struct disjunction<T1, T2, T3, Tn...>
    : std::conditional<T1::value, T1, disjunction<T2, T3, Tn...>>::type {};

template <typename...>
struct conjunction;

template <>
struct conjunction<>: std::true_type {};

template <typename T1>
struct conjunction<T1>: T1 {};

template <typename T1, typename T2>
struct conjunction<T1, T2>: std::conditional<T1::value, T2, T1>::type {};

template <typename T1, typename T2, typename T3, typename... Tn>
struct conjunction<T1, T2, T3, Tn...>
    : std::conditional<T1::value, conjunction<T2, T3, Tn...>, T1>::type {};

template <typename Tp>
struct negation: std::integral_constant<bool, !Tp::value> {};

template <bool Value, typename Tp = void>
using enable_if_t = typename std::enable_if<Value, Tp>::type;

template <typename Tp>
using decay_t = typename std::decay<Tp>::type;

template <typename... Tn>
using Requires = enable_if_t<conjunction<Tn...>::value, bool>;

template <typename Tp>
using remove_cv_t = typename std::remove_cv<Tp>::type;

template <typename Tp>
using remove_cvref = std::remove_cv<typename std::remove_reference<Tp>::type>;

template <typename Tp>
using remove_cvref_t = typename remove_cvref<Tp>::type;

template <typename Tp>
using add_pointer_t = typename std::add_pointer<Tp>::type;

template <typename Tp>
struct is_reference_wrapper: std::false_type {};

template <typename Tp>
struct is_reference_wrapper<std::reference_wrapper<Tp>>: std::true_type {};

#if 1
template <typename>
struct is_function: std::false_type {};

// Specialization for regular functions
template <typename R, typename... Args>
struct is_function<R(Args...)>: std::true_type {};

// Specialization for variadic functions such as printf
template <typename R, typename... Args>
struct is_function<R(Args......)>: std::true_type {};

// Specialization for function types that have cv-qualifiers
template <typename R, typename... Args>
struct is_function<R(Args...) const>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) volatile>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const volatile>: std::true_type {};

// Specialization for function types that have ref-qualifiers
template <typename R, typename... Args>
struct is_function<R(Args...) &>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) &>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) volatile&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const volatile&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) &&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const&&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile&&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile&&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) &&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const&&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) volatile&&>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const volatile&&>: std::true_type {};

#if __cplusplus >= 2017003L
// Specialization for noexcept version of all the above in the C++17 and later.
template <typename R, typename... Args>
struct is_function<R(Args...) noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) volatile noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const volatile noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) & noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) & noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) volatile& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const volatile& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) && noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const&& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) volatile&& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args...) const volatile&& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) && noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const&& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) volatile&& noexcept>: std::true_type {};

template <typename R, typename... Args>
struct is_function<R(Args......) const volatile&& noexcept>: std::true_type {};
#endif // __cplusplus >= 201703L
#else
template <typename Tp>
struct is_function
    : std::integral_constant<
        bool,
        !(std::is_reference<Tp>::value ||
          std::is_const<typename std::add_const<Tp>::type>::value)> {};
#endif

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
        type_traits_internal::is_trivially_copy_constructible_object<Tp>,
        std::is_lvalue_reference<Tp>
    >::type::type {
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
        type_traits_internal::is_trivially_move_constructible_object<Tp>,
        std::is_reference<Tp>
    >::type::type {
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
        std::is_copy_assignable<Tp>::value
    > {
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
        std::is_move_assignable<type_traits_internal::SingleMemberUnion<Tp>>,
        type_traits_internal::is_trivially_move_assignable_reference<Tp>
    >::type::type {
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

template <typename...>
struct Void { using type = void; };

template <typename... Args>
using void_t = typename Void<Args...>::type;

namespace type_traits_internal {

using std::swap; // Important for the traits.

template <typename Tp>
struct is_swappable_helper {
  template<typename Up,
        typename = decltype(swap(std::declval<Up&>(), std::declval<Up&>()))>
  static std::true_type fn(int);

  template<typename>
  static std::false_type fn(...);

  using type = decltype(fn<Tp>(0));
};

template <typename Tp>
struct is_nothrow_swappable_helper {
  template <typename Up>
  static std::integral_constant<
      bool, noexcept(swap(std::declval<Up&>(), std::declval<Up&>()))> fn(int);

  template <typename>
  static std::false_type fn(...);

  using type = decltype(fn<Tp>(0));
};

} // namespace type_traits_internal

template <typename Tp>
struct is_swappable
    : type_traits_internal::is_swappable_helper<Tp>::type {};

// Some standard library implementations are broken in that they do not
// constrain `std::swap`. This will effectively tell us if we are dealing
// with on of those implementations.
using StdSwapIsUnconstrained = is_swappable<void()>;

template <typename Tp>
struct is_nothrow_swappable
    : type_traits_internal::is_nothrow_swappable_helper<Tp>::type {};

// integer_sequence
// The class template integer_sequence represents a compile-time sequence
// of integers. When used as an argument to a function template,
// the parameter pack Ints can be deduced and used in pack expansion.
template <typename Tp, Tp... Idx>
struct integer_sequence {
  using value_type = Tp;
  static constexpr std::size_t size() noexcept { return sizeof...(Idx); }
};

#if POLLY_HAS_BUILTIN(__make_integer_seq)
template <typename Tp, Tp Num>
using make_integer_sequence = __make_integer_seq<integer_sequence, Tp, Num>;
#else // POLLY_HAS_BUILTIN(__make_integer_seq)
namespace type_traits_internal {
template <typename Seq, std::size_t Num, std::size_t Rem>
struct integer_pack;

template <typename Tp, Tp... Idx, std::size_t Num>
struct integer_pack<integer_sequence<Tp, Idx...>, Num, 0> {
  using type = integer_sequence<Tp, Idx..., (Idx + Num)...>;
};

template <typename Tp, Tp... Idx, std::size_t Num>
struct integer_pack<integer_sequence<Tp, Idx...>, Num, 1> {
  using type = integer_sequence<Tp, Idx..., (Idx + Num)..., Num * 2>;
};

template <typename Tp, std::size_t Num>
struct make_integer_sequence {
  using type = typename integer_pack<
      typename make_integer_sequence<Tp, Num / 2>::type, Num / 2, Num % 2
  >::type;
};

template <typename Tp>
struct make_integer_sequence<Tp, 0> {
  using type = integer_sequence<Tp>;
};

} // namespace type_traits_internal

template <typename Tp, Tp Num>
using make_integer_sequence =
        typename type_traits_internal::make_integer_sequence<Tp, Num>::type;
#endif // POLLY_HAS_BUILTIN(__make_integer_seq)

template <std::size_t... Idx>
using index_sequence = integer_sequence<std::size_t, Idx...>;

template <std::size_t Num>
using make_index_sequence = make_integer_sequence<std::size_t, Num>;

template <typename... Types>
using index_sequence_for = make_index_sequence<sizeof...(Types)>;

// type_identity
template <typename Tp>
struct type_identity {
  using type = Tp;
};

template <typename Tp>
using type_identity_t = typename type_identity<Tp>::type;

} // namespace polly