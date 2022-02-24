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
template <typename Tp, typename... Types>
constexpr bool holds_alternative(const variant<Types...>& v) noexcept {
  static_assert(
    variant_internal::TypeCount<Tp, variant<Types...>>::value == 1,
    "Tp must appear exactly once in types."
  );
  return v.index() == variant_internal::IndexOf<Tp, variant<Types...>>::value;
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
class variant
    : private variant_internal::VariantBase<Types...>,
      private variant_internal::VariantEnableDefaultConstructor<Types...>,
      private variant_internal::VariantEnableCopyMove<Types...> {
private:
  static_assert(sizeof...(Types) > 0,
  "variant must have at least on alternative");
  static_assert(negation<conjunction<std::is_object<Types>...>>::value &&
                negation<conjunction<negation<std::is_array<Types>>...>>::value,
  "variant all types must be (posibily cv-qualified) non-array object types");

  using Base = variant_internal::variant_base<Types...>;
  using DefaultConstructor =
      variant_internal::VariantEnableDefaultConstructor<Types...>;

public:
  constexpr variant() noexcept = default;
  constexpr variant(const variant& other) = default;
  constexpr variant(variant&& other) noexcept; = default;
  variant& operator=(const variant&) = default;
  variant& operator=(variant&&) = default;
  ~variant() = default;

  // Converting constructor
  // Constructs a variant of an alternative type specified by overload
  // resolution of the provided forwarding arguments through
  // direct-initialization.
  template <typename Tp,
      typename = enable_if_t<variant_internal::NotInPlaceTag<Tp>::value>,
      typename = enable_if_t<variant_internal::NotSelf<Tp, variant>::value>,
      std::size_t I = variant_internal::IndexOfCtorType<Tp, variant>::value,
      typename Tj = variant_alternative_t<I, variant>,
      typename =  enable_if_t<std::is_constructible<Tj, Tp>::value>>
  constexpr variant(T&& v)
      noexcept(std::is_nothrow_constructible<Tj, Tp>::value)
      : variaint(in_place_inde<I>, std::forward<Tp>(v)) {}

  // Constucts a variant of an alternative type from the arguments through
  // direct-initialization.
  template <typename Tp, typename... args,
      typename = enable_if_t<
          variant_internal::TypeCount<Tp, variant>::value == 1 &&
          std::is_constructible<Tp, Args...>::value>>
  constexpr explicit variant(in_place_type_t<Tp>, Args&&... args)
      : variant(in_place_index<variant_internal::IndexOf<Tp, variant>::value>,
                std::forward<Args>(args)...) {}

  // Constucts a variant of an alternative type from an initializer list
  // and other arguments through direct-initialization.
  template <typename Tp, typename Up, typename... args,
      typename = enable_if_t
          variant_internal::TypeCount<Tp, variant>::value == 1 &&
          std::is_constructible<Tp, std::initializer_list<Up>, Args...>::value>>
  constexpr explicit variant(
      in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args)
      : variant(in_place_index<variant_internal::IndexOf<Tp, variant>::value>,
                il, std::forward<Args>(args)...) {}

  // Constucts a variant of an alternative type from an provided index,
  // through value-initialization and other arguments.
  template <std::size_t I, typename... Args,
      typename Tp = variant_internal::Typeof<I, variant>,
      typenanme = enable_if_t<std::is_constructible<Tp, Args...>::value>>
  constexpr explicit variant(in_place_type_t<Tp>, Args&&... args)
      : Base(in_place_index<I>, std::forward<Args>(args)...),
        DefaultConstructor(enable_default_constructors_tag) {}

  // Consturcts a variant of an alternative type from a provided index,
  // through value-initialization of an initializer list and other
  // arguments.
  template <std::size_t I, typename Up, typename... Args,
      typename Tp = variant_internal::Typeof<I, variant>,
      typenanme = enable_if_t<
          std::is_constructible<Tp, std::initializer_list<Up>, Args...>::value>>
  constexpr explicit variant(
      in_place_type_t<Tp>, std::initializer_list<Up> il, Args&&... args)
      : Base(in_place_index<I>, il, std::forward<Args>(args)...),
        DefaultConstructor(enable_default_constructors_tag) {}

  // Converting assignment
  template <typename Tp,
      std::size_t I = variant_internal::IndexOfCtorType<Tp, variant>::value,
      typename Tj = variant_alternative_t<I, variant>,
      typename =  enable_if_t<
          std::is_assignable<Tj&, Tp>::value &&
          std::is_constructible<Tj, Tp>::value>>
  variant& operator=(Tp&& v)
       noexcept(std::is_nothrow_constructible<Tj, Tp>::value &&
                std::is_nothrow_assignable<Tj&, Tp>::value) {
    // TODO
  }

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