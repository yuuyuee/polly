#pragma once

#include "stubs/type_traits.h"
#include "stubs/utility.h"
#include "stubs/assert.h"

namespace polly {
template <typename Tp>
class optional;

namespace optional {
namespace optional_internal {

struct empty_type {};

// This class stores the data in optional<T>.
// It is specialized based on whether T is trivially destructible.
// This is the specialization for trivially destructible type.
template <typename Tp, bool = std::is_trivially_destructible<Tp>::type>
class optional_data_dtor_impl {
protected:
  void destroy() noexcept {
      engaged_ = false;
  }

  bool engaged_;
  union {
    empty_type empty_;
    Tp value_;
  };
};

// This is the specialization for non trivially destructible type.
template <typename Tp>
class optional_data_dtor_impl<Tp, false> {
protected:
  ~optional_data_destructor_impl() {}

  void destroy() noexcept {
    if (engaged_) {
      engaged_ = false;
      data_.~T();
    }
  }

  bool engaged_;
  union {
    empty_type empty_;
    Tp value_;
  };
};

// THis class template manages constructioin/destruction/assignment of
// the contained value for a polly::optional.
template <typename Tp,
          typename store_type = typename std::remove_const<Tp>::type>
class optional_data_base: public optional_data_dtor_impl<store_type> {
protected:
  using optional_data_dtor_base<store_type>::optional_data_dtor_base;

  constexpr optional_data_base() noexcept
      : engaged_(false), empty_() {}

  template<typename... Args>
  constexpr optional_data_base(in_place_t, Args&&... args)
      : engaged_(true), value_(std::forward<Args>(args)...) {}

  // non-trivial copy constructor
  optional_data_base(bool, const optional_data_base& other) {
    if (other.engaged_)
      this->construct(other.value_);
  }

  // no-trivial move constructor
  optional_data_base(bool, optional_data_base&& other) {
    if (other.engaged_)
      this->construct(std::move(other.value_));
  }

  // non-trivial copy assignment
  void copy_assign(const optional_data_base& other) {
    if (this->engaged_ && other.engaged_) {
      this->value_ = other.value;
    } else {
      if (other.engaged_) {
        this->construct(other.value_);
      } else {
        this->reset();
      }
    }
  }

  // non-trivial move assignment
  void move_assign(optional_data_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    if (this->engaged_ && other.engaged_) {
      this->value_ = std::move(other.value_);
    } else {
      if (other.engaged_) {
        this->construct(std::move(other.value_));
      } else {
        this->reset();
      }
    }
  }

  template<typename... Args>
  void construct(Args&&... args)
      noexcept(std::is_nothrow_constructible<store_type, Args...>::value) {
    this->engaged_ = true;
    ::new (std::addressof(this->value_))
        store_type(std::forward<Args>(args)...);
  }

  void reset() noexcept { destroy(); }
  constexpr store_type& get() & noexcept { return this->value_; }
  constexpr bool is_engaged() const noexcept { return this->engaged_; }
};

template <typename Tp,
          // has trivially destructor
          bool = std::is_trivially_destructible<Tp>::value,
          // has trivially copy assignment&constructor
          bool = polly::is_trivially_copy_assignable<Tp>::value &&
              polly::is_trivially_copy_constructible<Tp>::value,
          // has trivially move assignment&constructor
          bool = polly::is_trivially_move_assignable<Tp>::value &&
              polly::is_trivially_move_constructible<Tp>::value
  >
class optional_base;

// optional_data with trivial destroy, copy and move.
template <typename Tp>
class optional_base<Tp, true, true, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;
};

// optional_data with non-trivial copy construction and assignment.
template <typename Tp>
class optional_base<Tp, true, false, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_base(const optional_base& other)
      : optional_data_base<Tp>(other.engaged_, other) {}

  optional_base& operator=(const optional_base& other) {
    this->copy_assign(other);
    return *this;
  }
};

// optional_data with non-trivial move construction and assignment.
template <typename Tp>
class optional_base<Tp, true, true, false>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_base(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>)
      : optional_data_base<Tp>(other.engaged_, std::move(other.get())) {}

  optional_base& operator=(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial copy/move construction and assignment.
template <typename Tp>
class optional_base<Tp, true, false, false>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_base(const optional_base& other)
      : optional_data_base<Tp>(other.engaged_, other.get()) {}

  optional_base& operator=(const optional_base& other) {
    this->copy_assign(other);
    return *this;
  }

  optional_base(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>)
      : optional_data_base<Tp>(other.engaged_, std::move(other.get())) {}

  optional_base& operator=(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial destroy
template <typename Tp, bool Copy, bool Move>
class optional_base<Tp, false, Copy, Move>
    : public optional_base<Tp, true, Copy, Move> {
public:
  using optional_data_base<Tp>::optional_data_base;
  ~optional_base() { this->reset(); }
};

template <typename Tp, typename Up>
using converts_from_optional = std::integral_constant<
  bool,
  std::is_constructible<Tp, const optional<Up>&>::value  ||
  std::is_constructible<Tp, optional<Up>&>::value        ||
  std::is_constructible<Tp, const optional<Up>&&>::value ||
  std::is_constructible<Tp, optional<Up>&&>::value       ||
  std::is_convertible<const optional<Up>&, Tp>::value    ||
  std::is_convertible<optional<Up>&, Tp>::value          ||
  std::is_convertible<const optional<Up>&&, Tp>::value   ||
  std::is_convertible<optional<Up>&&, Tp>::value
>;

template <typename Tp, typename Up>
using assigns_from_optional = std::integral_constant<
  bool,
  std::is_assignable<Tp&, const optional<Up>&>::value    ||
  std::is_assignable<Tp&, optional<Up>&>::value          ||
  std::is_assignable<Tp&, const optional<Up>&&>::value   ||
  std::is_assignable<Tp&, optional<Up>&&>::value
>;

} // optional_internal
} // optional
} // namespace polly