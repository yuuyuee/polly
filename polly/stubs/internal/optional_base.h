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
protected:
  using store_type = typename std::remove_const<Tp>::type;

  optional_data_base() = default;
  ~optional_data_base() = default;

  template<typename... Args>
  constexpr explicit optional_data_base(in_place_t tag, Args&&... args)
      :  engaged_(true), payload_(tag, std::forward<Args>(args)...) {}

  template<typename Up, typename... Args>
  constexpr explicit optional_data_base(in_place_t tag, std::initializer_list<Up> il, Args&&... args)
      :  engaged_(true), payload_(tag, il, std::forward<Args>(args)...) {}

  // Copy/move constructor is only used to when the contained value is
  // trivially copy constructible.
  optional_data_base(const optional_data_base&) = default;
  optional_data_base(optional_data_base&&) = default;
  optional_data_base& operator=(const optional_data_base&) = default;
  optional_data_base& operator=(optional_data_base&&) = default;

  // Used to perform non-trivial copy constructor.
  optional_data_base(bool, const optional_data_base& other) {
    if (other.engaged_)
      construct(other.payload_.value_);
  }

  // Used to perform non-trivial move constructor.
  optional_data_base(bool, optional_data_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value) {
    if (other.engaged_)
      construct(std::move(other.payload_.value_));
  }

  // Used to perform non-trivial copy assignment constructor.
  void copy_assign(const optional_data_base& other) {
    if (engaged_ && other.engaged_) {
      payload_.value_ = other.payload_.value_;
    } else {
      if (other.engaged_) {
        construct(other.payload_.value_);
      } else {
        destroy();
      }
    }
  }

  // Used to perform non-trivial move assignment constructor.
  void move_assign(optional_data_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    if (engaged_ && other.engaged_) {
      payload_.value_ = std::move(other.payload_.value_);
    } else {
      if (other.engaged_) {
        construct(std::move(other.payload_.value_));
      } else {
        destroy();
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

  store_type& get() & noexcept { return payload_.value_; }
  constexpr const store_type& get() const & noexcept { return payload_.value_; }
  constexpr bool is_engaged() const noexcept { return engaged_; }

  void reset() noexcept {
    if (is_engaged())
      destroy();
  }

private:
  bool engaged_{false};

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
    constexpr explicit Storage(in_place_t, std::initializer_list<Vp> il, Args&&... args)
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
    constexpr explicit Storage(in_place_t, std::initializer_list<Vp> il, Args&&... args)
        : value_(il, std::forward<Args>(args)...) {}

    union {
      Empty empty_;
      Up value_;
    };
  };

  Storage<store_type> payload_;
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
> class optional_base;

// optional_data with trivial destroy, copy and move.
template <typename Tp>
class optional_base<Tp, true, true, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_base() = default;
};

// optional_data with non-trivial copy construction and assignment.
template <typename Tp>
class optional_base<Tp, true, false, true>
    : public optional_data_base<Tp> {
public:
  using optional_data_base<Tp>::optional_data_base;

  optional_base() = default;
  ~optional_base() = default;

  optional_base(optional_base&&) = default;
  optional_base& operator=(optional_base&&) = default;

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

  optional_base() = default;
  ~optional_base() = default;
  optional_base(const optional_base&) = default;
  optional_base& operator=(const optional_base&) = default;

  optional_base(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value)
      : optional_data_base<Tp>(other.is_engaged(), std::move(other)) {}

  optional_base& operator=(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
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

  optional_base() = default;
  ~optional_base() = default;

  optional_base(const optional_base& other)
      : optional_data_base<Tp>(other.is_engaged(), other) {}

  optional_base& operator=(const optional_base& other) {
    this->copy_assign(other);
    return *this;
  }

  optional_base(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value)
      : optional_data_base<Tp>(other.is_engaged(), std::move(other)) {}

  optional_base& operator=(optional_base&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    this->move_assign(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial destroy
template <typename Tp, bool Copy, bool Move>
class optional_base<Tp, false, Copy, Move>
    : public optional_base<Tp, true, Copy, Move> {
public:
  using optional_base<Tp, true, false, false>::optional_base;

  optional_base() = default;
  optional_base(const optional_base&) = default;
  optional_base(optional_base&&) = default;
  optional_base& operator=(optional_base&&) = default;
  optional_base& operator=(const optional_base&) = default;

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