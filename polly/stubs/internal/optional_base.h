#pragma once

#include <functional>
#include <initializer_list>

#include "stubs/type_traits.h"
#include "stubs/utility.h"
#include "stubs/assert.h"

namespace polly {
template <typename Tp>
class optional;

namespace optional_internal {
// THis class template manages constructioin/destruction/assignment of
// the contained value for a polly::optional.
template <typename Tp>
class optional_data_base {
public:
  using store_type = typename std::remove_const<Tp>::type;

  optional_data_base() = default;
  ~optional_data_base() = default;

  template<typename... Args>
  constexpr explicit optional_data_base(in_place_t tag, Args&&... args)
      :  payload_(tag, std::forward<Args>(args)...), engaged_(true) {}

  template<typename Up, typename... Args>
  constexpr explicit optional_data_base(
      std::initializer_list<Up> il, Args&&... args)
      :  payload_(il, std::forward<Args>(args)...), engaged_(true) {}

  // Trivially copy/move constructor
  optional_data_base(const optional_data_base&) = default;
  optional_data_base(optional_data_base&&) = default;
  optional_data_base& operator=(const optional_data_base&) = default;
  optional_data_base& operator=(optional_data_base&&) = default;

  // Non-trivially copy/move constructor
  optional_data_base(bool, const optional_data_base& other) {
    if (other.engaged_)
      construct(other.payload_.value_);
  }

  optional_data_base(bool, optional_data_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value) {
    if (other.engaged_)
      construct(std::move(other.payload_.value_));
  }

  void copy_assign(const optional_data_base& other) {
    if (engaged_ && other.engaged_) {
      payload_.value_ = other.payload_.value_;
    } else {
      if (other.engaged_) {
        construct(other.payload_.value_);
      } else {
        reset();
      }
    }
  }

  void move_assign(optional_data_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    if (engaged_ && other.engaged_) {
      payload_.value_ = std::move(other.payload_.value_);
    } else {
      if (other.engaged_) {
        construct(std::move(other.payload_.value_));
      } else {
        reset();
      }
    }
  }

  template<typename... Args>
  void construct(Args&&... args)
      noexcept(std::is_nothrow_constructible<store_type, Args...>::value) {
    ::new(std::addressof(payload_.empty_)) store_type(std::forward<Args>(args)...);
    this->engaged_ = true;
  }

  void destroy() noexcept {
    payload_.value_.~store_type();
    engaged_ = false;
  }

  Tp& get() & noexcept { return payload_.value_; }
  constexpr const Tp& get() const & noexcept { return payload_.value_; }
  constexpr bool is_engaged() const noexcept { return engaged_; }

  void reset() noexcept {
    if (engaged_)
      destroy();
  }

  struct Empty {};

  // This class stores the data in optional<T>.
  // It is specialized based on whether T is trivially destructible.
  template <typename Up, bool = std::is_trivially_destructible<Up>::value>
  struct Storage {
  public:
    constexpr Storage() noexcept : empty_() {}

    template <typename... Args>
    constexpr explicit Storage(in_place_t, Args&&... args)
        : value_(std::forward<Args>(args)...) {}

    template <typename Vp, typename... Args>
    constexpr explicit Storage(std::initializer_list<Vp> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...) {}

    ~Storage() {}

    union {
      Empty empty_;
      Up value_;
    };
  };

  // This is the specialization for non trivially destructible type.
  template <typename Up>
  class Storage<Up, true> {
  public:
    constexpr Storage() noexcept : empty_() {}

    template <typename... Args>
    constexpr Storage(in_place_t, Args&&... args)
        : value_(std::forward<Args>(args)...) {}

    template <typename Vp, typename... Args>
    constexpr explicit Storage(std::initializer_list<Vp> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...) {}

    union {
      Empty empty_;
      Up value_;
    };
  };

  Storage<store_type> payload_;
  bool engaged_{false};
};

template <
    typename Tp,
    // has trivially destructor
    bool = std::is_trivially_destructible<Tp>::value,
    // has trivially copy assignment&constructor
    bool = polly::is_trivially_copy_assignable<Tp>::value &&
        polly::is_trivially_copy_constructible<Tp>::value,
    // has trivially move assignment&constructor
    bool = polly::is_trivially_move_assignable<Tp>::value &&
        polly::is_trivially_move_constructible<Tp>::value
> class optional_data;

// optional_data with trivial destroy, copy and move.
template <typename Tp>
class optional_data<Tp, true, true, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_data() = default;
};

// optional_data with non-trivial copy construction and assignment.
template <typename Tp>
class optional_data<Tp, true, false, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_data() = default;
  ~optional_data() = default;

  optional_data(const optional_data& other) = default;
  optional_data(optional_data&&) = default;
  optional_data& operator=(optional_data&&) = default;

  optional_data& operator=(const optional_data& other) {
    this->copy_assign(other);
    return *this;
  }
};

// optional_data with non-trivial move construction and assignment.
template <typename Tp>
class optional_data<Tp, true, true, false>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_data() = default;
  ~optional_data() = default;
  optional_data(const optional_data&) = default;
  optional_data& operator=(const optional_data&) = default;
  optional_data(optional_data&& other) = default;

  optional_data& operator=(optional_data&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial copy/move construction and assignment.
template <typename Tp>
class optional_data<Tp, true, false, false>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_data() = default;
  ~optional_data() = default;
  optional_data(const optional_data& other) = default;
  optional_data(optional_data&& other) = default;

  optional_data& operator=(const optional_data& other) {
    this->copy_assign(other);
    return *this;
  }

  optional_data& operator=(optional_data&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial destroy
template <typename Tp, bool Copy, bool Move>
class optional_data<Tp, false, Copy, Move>
    : public optional_data<Tp, true, Copy, Move> {
public:
  using optional_data<Tp, true, false, false>::optional_data;

  optional_data() = default;
  optional_data(const optional_data&) = default;
  optional_data(optional_data&&) = default;
  optional_data& operator=(optional_data&&) = default;
  optional_data& operator=(const optional_data&) = default;

  ~optional_data() { this->reset(); }
};

// Common member functions for optional_base
template <typename Tp, typename Dp>
class optional_base_impl {
protected:
  using store_type = typename std::remove_const<Tp>::type;

  template <typename... Args>
  void construct(Args&&... args)
      noexcept(std::is_nothrow_constructible<store_type, Args...>::value) {
    static_cast<Dp*>(this)->data_.construct(std::forward<Args>(args)...);
  }

  void destruct() noexcept {
    static_cast<Dp*>(this)->data_.destroy();
  }

  void reset() noexcept {
    static_cast<Dp*>(this)->data_.reset();
  }

  constexpr bool is_engaged() const noexcept {
    return static_cast<const Dp*>(this)->data_.is_engaged();
  }

  Tp& get() & noexcept {
    return static_cast<Dp*>(this)->data_.get();
  }

  constexpr const Tp& get() const & noexcept {
    return static_cast<const Dp*>(this)->data_.get();
  }
};

template <
    typename Tp,
    bool = is_trivially_copy_constructible<Tp>::value,
    bool = is_trivially_move_constructible<Tp>::value>
class optional_base;

template <typename Tp>
class optional_base<Tp, false, false>
    : public optional_base_impl<Tp, optional_base<Tp>> {
public:
  constexpr optional_base() = default;

  template <typename... Args>
  constexpr explicit optional_base(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit optional_base(
      in_place_t, std::initializer_list<Up> il, Args&&... args)
      : data_(in_place, il, std::forward<Args>(args)...) {}

  constexpr optional_base(const optional_base& other)
      : data_(other.data_.is_engaged(), other.data_) {}

  constexpr optional_base(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value)
      : data_(other.data_.is_engaged(), std::move(other.data_)) {}

  optional_base& operator=(const optional_base&) = default;
  optional_base& operator=(optional_base&&) = default;

  optional_data<Tp> data_;
};

template <typename Tp>
class optional_base<Tp, false, true>
    : public optional_base_impl<Tp, optional_base<Tp>> {
public:
  constexpr optional_base() = default;

  template <typename... Args>
  constexpr explicit optional_base(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit optional_base(
      in_place_t, std::initializer_list<Up> il, Args&&... args)
      : data_(in_place, il, std::forward<Args>(args)...) {}

  constexpr optional_base(const optional_base& other)
      : data_(other.data_.is_engaged(), other.data_) {}

  constexpr optional_base(optional_base&& other) = default;

  optional_base& operator=(const optional_base&) = default;
  optional_base& operator=(optional_base&&) = default;

  optional_data<Tp> data_;
};

template <typename Tp>
class optional_base<Tp, true, false>
    : public optional_base_impl<Tp, optional_base<Tp>> {
public:
  constexpr optional_base() = default;

  template <typename... Args>
  constexpr explicit optional_base(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit optional_base(
      in_place_t tag, std::initializer_list<Up> il, Args&&... args)
      : data_(tag, il, std::forward<Args>(args)...) {}

  constexpr optional_base(const optional_base& other) = default;

  constexpr optional_base(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value)
      : data_(other.data_.is_engaged(), std::move(other.data_)) {}

  optional_base& operator=(const optional_base&) = default;
  optional_base& operator=(optional_base&&) = default;

  optional_data<Tp> data_;
};

template <typename Tp>
class optional_base<Tp, true, true>
    : public optional_base_impl<Tp, optional_base<Tp>> {
public:
  constexpr optional_base() = default;

  template <typename... Args>
  constexpr explicit optional_base(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit optional_base(
      in_place_t, std::initializer_list<Up> il, Args&&... args)
      : data_(in_place, il, std::forward<Args>(args)...) {}

  constexpr optional_base(const optional_base& other) = default;
  constexpr optional_base(optional_base&& other) = default;

  optional_base& operator=(const optional_base&) = default;
  optional_base& operator=(optional_base&&) = default;

  optional_data<Tp> data_;
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

// Helper function for checking whether an expression is convertible to bool.
bool convertible_to_bool(bool);

#define POLLY_OPTIONAL_CONVERTIBLE_BOOL(exp)  \
  decltype(::polly::optional_internal::convertible_to_bool(exp))

template <typename Tp, typename = size_t>
struct optional_hash_base {
  optional_hash_base() = delete;
  optional_hash_base(const optional_hash_base&) = delete;
  optional_hash_base(optional_hash_base&&) = delete;
  optional_hash_base& operator=(const optional_hash_base&) = delete;
  optional_hash_base& operator=(optional_hash_base&&) = delete;
};

#define POLLY_OPTION_HASH_FN                        \
  std::hash<typename std::remove_const<Tp>::type>{} \
      (std::declval<typename std::remove_const<Tp>::type>())

template <typename Tp>
struct optional_hash_base<Tp, decltype(POLLY_OPTION_HASH_FN)> {
  using argument_type = polly::optional<Tp>;
  using result_type = size_t;

  size_t operator()(const optional<Tp>& opt)
      const noexcept(noexcept(POLLY_OPTION_HASH_FN)) {
    constexpr size_t kMgicDisengagedHash = static_cast<size_t>(-3333);
    return opt ? std::hash<typename std::remove_const<Tp>::type>{}(*opt)
               : kMgicDisengagedHash;
  }
};

} // optional_internal
} // namespace polly