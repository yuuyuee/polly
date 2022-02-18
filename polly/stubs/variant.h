#pragma once

#include "stubs/internal/config.h"

#if defined(POLLY_HAVE_STD_VARIANT)
#include <variant>

namespace polly {
using std::vad_variant_access;
using std::get;
using std::get_if;
using std::holds_alternative;
using std::monostate;
using std::variant;
using std::variant_alternative;
using std::variant_alternative_t;
using std::variant_npos;
using std::variant_size;
using std::variant_size_v;
using std::visit;
} // namespace polly

#else // POLLY_HAVE_STD_VARIANT

#include <functional>
#include <new>
#include <exception>
#include <initializer_list>

#include "stubs/macros.h"
#include "stubs/type_traits.h"
#include "stubs/utility.h"

namespace polly {
template <typename...>
class variant;

// Helper objects
POLLY_INLINE_CONSTEXPR(size_t, variant_npos, static_cast<size_t>(-1));

// Helper classes

// monostate
// Unit type instended for use as a well-behaved empty alternative in variant.
// monostate serves as a first alternative type for a variant for which the
// first variant type is otherwise not default constructible.
struct monostate {};

constexpr bool operator<(monostate, monostate) noexcept { return false; }
constexpr bool operator>(monostate, monostate) noexcept { return false; }
constexpr bool operator<=(monostate, monostate) noexcept { return true; }
constexpr bool operator>=(monostate, monostate) noexcept { return true; }
constexpr bool operator==(monostate, monostate) noexcept { return true; }
constexpr bool operator!=(monostate, monostate) noexcept { return false; }

// bad_variant_access
// Exception thrown on invalid accesses to the value of a variant.
class bad_variant_access: public std::exception {
public:
  bad_variant_access() noexcept = default;
  virtual ~bad_variant_access() noexcept = default;

  virtual const char* what() const noexcept override {
    return "Bad variant access";
  }
};

// Throw delegate
[[noreturn]] inline void ThrowBadVariantAccess() {
#ifndef POLLY_HAVE_EXCEPTIONS
  POLLY_RAW_LOG(FATAL, bad_variant_access{}.what());
#else
  throw bad_variant_access{};
#endif
}

// variant_size
// Provides access to the number of alternatives in a possibly cv-qualified
// variant as a compile-time constant expression.
template <typename>
struct variant_size;

template <typename... Types>
struct variant_size<variant<Types...>>
    : public std::integral_constant<std::size_t, sizeof...(Types)> {};

// Specialization for const qualified variant.
template <typename Tp>
struct variant_size<const Tp>: public variant_size<Tp>::value {};

// Specialization for volatile qualified variant.
template <typename Tp>
struct variant_size<volatile Tp>: public variant_size<Tp>::value {};

// Specialization for const volatile qualified variant.
template <typename Tp>
struct variant_size<const volatile Tp>: public variant_size<Tp>::value {};

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

// Non-member functions

// visit
// TODO

// holds_alternative
// Checks if the variant holds the alternative type. The call is ill-formed
// if type does not appear exactly once in types.
namespace variant_internal {
template <typename Tp, typename Variant>
struct type_in;

template <typename Tp, typename Last>
struct type_in<Tp, variant<Last>>
    : public std::integral_constant<std::size_t, std::is_same<Tp, Last>::value> {};

template <typename Tp, typename First, typename... Last>
struct type_in<Tp, variant<First, Last...>>
    : public std::integral_constant<
        std::size_t,
        type_in<Tp, First>::value + type_in<Tp, Last...>::value> {};

template <typename Tp, typename Variant>
struct index_of: public std::integral_constan<std::size_t, 0> {};

template <typename Tp, typename First, typename... Last>
struct index_of<Tp, variant<Tp, First, Last...>>
    : public std::integral_constant<
        std::size_t,
        type_in<Tp, variant<First>>::value ? 0 : index_of<Tp, variant<Last...>>::value + 1
      > {};

} // namespace variant_internal

template <typename Tp, typename... Types>
constexpr bool holds_alternative(const variant<Types...>& v) noexcept {
  static_assert(
    variant_internal::type_in<Tp, variant<Types...>>::value,
    "Tp must appear exactly once in types."
  );
  return v.index() == variant_internal::index_of<Tp, variant<Types...>>::value;
}

// get
// Reads the value of the variant given the index or the unique alternative  type.
// Attempting to get value using a type that is not unique within the variant set
// of alternative types is a compile-time error. If the index of the alternative
// being specified is different from the index of the alternative that currently
// stored, throws `bad_variant_access`.
template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>>& get(
    variant<Types...>& v) {

}

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, const variant<Types...>>& get(
    const variant<Types...>& v) {

}

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, variant<Types...>>&& get(
    const variant<Types...>&& v) {

}

template <std::size_t I, typename... Types>
constexpr variant_alternative_t<I, const variant<Types...>>&& get(
    const variant<Types...>&& v) {

}

template <typename Tp, typename... Types>
constexpr Tp const& get(const variant<Types...>& v) {
  if (POLLY_EXPECT_TRUE(holds_alternative<Tp>(v))) {
    return get<variant_internal::index_of<Tp, variant<Types...>>::value>(v);
  }
  ThrowBadVariantAccess();
}

template <typename Tp, typename... Types>
constexpr Tp& get(variant<Types...>& v) {
  if (POLLY_EXPECT_TRUE(holds_alternative<Tp>(v))) {
    return get<variant_internal::index_of<Tp, variant<Types...>>::value>(v);
  }
  ThrowBadVariantAccess();
}

template <typename Tp, typename... Types>
constexpr Tp&& get(const variant<Types...>&& v) {
  if (POLLY_EXPECT_TRUE(holds_alternative<Tp>(v))) {
    return get<variant_internal::index_of<Tp, variant<Types...>>::value>(std::move(v));
  }
  ThrowBadVariantAccess();
}

template <typename Tp, typename... Types>
constexpr Tp const&& get(const variant<const Types...>&& v) {
  if (POLLY_EXPECT_TRUE(holds_alternative<Tp>(v))) {
    return get<variant_internal::index_of<Tp, variant<Types...>>::value>(std::move(v));
  }
  ThrowBadVariantAccess();
}

// get_if
// Obtains a pointer to the value of a pointed to variant given the index or
// the type, return null on error.
template <typename std::size_t I, typename... Types>
constexpr add_pointer_t<variant_alternative_t<I, variant<Types...>>>
    get_if(variant<Types...>* p) noexcept {

}

template <typename std::size_t I, typename... Types>
constexpr add_pointer_t<variant_alternative_t<I, const variant<Types...>>>
    get_if(const variant<Types...>* p) noexcept {

}

template <typename Tp, typename... Types>
constexpr add_pointer_t<Tp> get_if(variant<Types...>* p) noexcept {
  return get_if<variant_internal::index_of<Tp, variant<Types...>>::value>(p);
}

template <typename Tp, typename... Types>
constexpr add_pointer_t<const Tp> get_if(const variant<Types...>* p) noexcept {
  return get_if<variant_internal::index_of<Tp, variant<Types...>>::value>(p);
}

// compares operator
template <typename... Types>
constexpr bool operator==(const variant<Types...>& v, const variant<Types...>& w);

template <typename... Types>
constexpr bool operator!=(const variant<Types...>& v, const variant<Types...>& w);

template <typename... Types>
constexpr bool operator<(const variant<Types...>& v, const variant<Types...>& w);

template <typename... Types>
constexpr bool operator>(const variant<Types...>& v, const variant<Types...>& w);

template <typename... Types>
constexpr bool operator<=(const variant<Types...>& v, const variant<Types...>& w);

template <typename... Types>
constexpr bool operator>=(const variant<Types...>& v, const variant<Types...>& w);

// swap
template <typename... Types>
void swap(variant<Types...>& lhs, variant<Types...>& rhs) noexcept {

}

// variant
template <typename... Types>
class variant {
public:
  constexpr variant() noexcept;
  constexpr variant(const variant& other);
  constexpr variant(variant&& other) noexcept;

  template <typename Tp>
  constexpr variant(T&& v) noexcept;

  template <typename Tp, typename... args>
  constexpr explicit variant(in_place_type_t<Tp>, Args&&... args);

  template <typename Tp, typename Up, typename... args>
  constexpr explicit variant(in_place_type_t<Tp>, std::initializer_list<Up> il, &&... args);

  template <std::size_t I, typename... Args>
  constexpr explicit variant(in_place_type_t<Tp>, Args&&... args);

  template <std::size_t I, typename Up, typename... Args>
  constexpr explicit variant(in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args);

  ~variant();

  // Observers
  constexpr std::size_t index() const noexcept;

  constexpr bool valueless_by_exception() const noexcept;

  // Modifiers
  template <typename Tp, typename... Args>
  Tp& emplace(Args&&... args);

  template <typename Tp, typename Up, typename... Args>
  Tp& emplace(std::initializer_list<Up> il, &&... args);

  template <std::size_t I, typename... Args>
  variant_alternative_t<I, variant<Args...>>& emplace(Args&&... args);

  template <std::size_t I, typename Up, typename... Args>
  variant_alternative_t<I, variant<Args...>>& emplace(
      std::initializer_list<Up> il, Args&&... args);

  void swap(variant& rhs) noexcept;

private:
};



} // namespace polly
#endif // POLLY_HAVE_STD_VARIANT