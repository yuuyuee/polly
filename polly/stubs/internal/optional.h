#pragma once

#include "stubs/type_traits.h"
#include "stubs/utility.h"
#include "stubs/assert.h"

namespace polly {
template<typename Tp>
class optional;

namespace optional_internal {

struct empty_type {};

// This class stores the data in optional<T>.
// It is specialized based on whether T is trivially destructible.
// This is the specialization for trivially destructible type.
template<typename Tp, bool = std::is_trivially_destructible<Tp>::type>
struct storage {
  constexpr storage() noexcept: empty_() {}

  template<typename... Args>
  constexpr storage(in_place_t, Args&&... args)
      : value_(std::forward<Args>(args)...) {}

  constexpr Tp& get() noexcept { return value_; }

  union {
    empty_type empty_;
    Tp value_;
  };
};

// This is the specialization for non trivially destructible type.
template<typename Tp>
struct storage<Tp, false> {
  constexpr storage() noexcept: empty_() {}

  template<typename... Args>
  constexpr storage(in_place_t, Args&&... args)
      : value_(std::forward<Args>(args)...) {}

  // User-provided destructor is needed when Tp has non-trivial destructor.
  ~storage() {}

  union {
    empty_type empty_;
    Tp value_;
  };
};

// THis class template manages constructioin/destruction/assignment of
// the contained value for a polly::optional.
template<typename Tp>
class optional_data_base {
protected:
  using store_type = std::remove_const<Tp>::type;

  optional_data_base() = default;
  ~optional_data_base() = default;

  template<typename... Args>
  constexpr explicit optional_data_base(in_place_t tag, Args&&... args)
      : engaged_(true), data_(tag, std::forward<Args>(args)...) {}

  constexpr optional_data_base(const optional_data_base&) = default;
  constexpr optional_data_base(optional_data_base&&) = default;

  optional_data_base& operator=(const optional_data_base&) = default;
  optional_data_base& operator=(optional_data_base&&) = default;

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
      noexcept(std::is_nothrow_move_constructible<store_type>::value &&
               std::is_nothrow_move_assignable<store_type>::value) {
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
    engaged_ = true;
    ::new (std::addressof(data_.value_)) store_type(std::forward<Args>(args)...);
  }

  void destroy() noexcept {
    engaged_ = false;
    data_.value_.~store_type();
  }

  void reset() noexcept {
    if (engaged_)
      destroy();
  }

  bool engaged_{false};
  storage<store_type> data_;
};

template<typename Tp,
        // has trivially destructor
        bool = std::is_trivially_destructible<Tp>::value,
        // has trivially copy assignment&constructor
        bool = polly::is_trivially_copy_assignable<Tp>::value &&
              polly::is_trivially_copy_constructible<Tp>::value,
        // has trivially move assignment&constructor
        bool = polly::is_trivially_move_assignable<Tp>::value &&
              polly::is_trivially_move_constructible<Tp>::value
  >
class optional_data;

// optional_data with trivial destroy, copy and move.
template<typename Tp>
class optional_data<Tp, true, true, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;
  optional_data() = default;
};

// optional_data with non-trivial copy construction and assignment.
template<typename Tp>
class optional_data<Tp, true, false, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;
  optional_data() = default;
  ~optional_data() = default;

  optional_data(const optional_data& other)
      : optional_data_base<Tp>(other.engaged_, other.) {}

  optional_data& operator=(const optional_data& other) {
    this->copy_assign(other);
    return *this;
  }

  optional_data(optional_data&&) = default;
  optional_data& operator=(optional_data&&) = default;
};

// optional_data with non-trivial move construction and assignment.
template<typename Tp>
class optional_data<Tp, true, true, false>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;
  optional_data() = default;
  ~optional_data() = default;

  optional_data(const optional_data&) = default;
  optional_data& operator=(const optional_data&) = default;

  optional_data(optional_data&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>)
      : optional_data_base<Tp>(other.engaged_, std::move(other.get())) {}

  optional_data& operator=(optional_data&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial copy/move construction and assignment.
template<typename Tp>
class optional_data<Tp, true, false, false>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;
  optional_data() = default;
  ~optional_data() = default;

  optional_data(const optional_data& other)
      : optional_data_base<Tp>(other.engaged_, other.get()) {}

  optional_data& operator=(const optional_data& other) {
    this->copy_assign(other);
    return *this;
  }

  optional_data(optional_data&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>)
      : optional_data_base<Tp>(other.engaged_, std::move(other.get())) {}

  optional_data& operator=(optional_data&& other)
      noexcept(std::is_nothrow_move_constructible<Tp> &&
               std::is_nothrow_move_assignable<Tp>) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial destroy
template<typename Tp, bool Copy, bool Move>
class optional_data<Tp, false, Copy, Move>
    : public optional_data<Tp, true, Copy, Move> {
public:
  using optional_data_base<Tp>::optional_data_base;
  optional_data() = default;
  ~optional_data() { this->reset(); }

  optional_data(const optional_data&) = default;
  optional_data& operator=(const optional_data&) = default;

  optional_data(optional_data&&) = default;
  optional_data& operator=(optional_data&&) = default;
};

// provided common operators for optional_base<T> to avoid repeating these
// member functions in each specialization.
template<typename Tp, typename Dp>
class optional_base_impl {
protected:
  using store_type = std::remove_const<Tp>::type;

  template<typename... Args>
  constexpr void construct(Args&&... args)
      noexcept(std::is_nothrow_constructible<store_type, Args...>) {
    static_cast<Dp*>(this)->data_.construct(std::forward<Args>(args)...);
  }

  constexpr void destroy() noexcept {
    static_cast<Dp*>(this)->data_.destroy();
  }

  constexpr void reset() noexcept {
    static_cast<Dp*>(this)->data_.reset();
  }

  constexpr bool is_engaged() const noexcept {
    return static_cast<Dp*>(this)->data_.is_engaged_;
  }

  constexpr Tp& get() noexcept {
    return static_cast<Dp*>(this)->data_.value_;
  }
};

template<
  typename Tp,
  bool = is_trivially_copy_assignable<Tp>::value,
  bool = is_trivially_move_constructible<Tp>::value>
class optional_base: public optional_base_impl<Tp, optional_base<Tp>> {
public:
  constexpr optional_base() = default;

  template<
      typename... Args,
      typename = typenamem std::enable_if<
          std::is_constructible<Tp, Args...>::value
      >::type>
  constexpr explicit optionial_base(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  optional_data<Tp> data_;
};

} // optional_internal


} // namespace polly