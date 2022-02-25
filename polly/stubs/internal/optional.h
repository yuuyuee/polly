#pragma once

#include "stubs/internal/config.h"

#include <functional>
#include <initializer_list>

#include "stubs/type_traits.h"
#include "stubs/utility.h"
#include "stubs/internal/enable_special_members.h"

namespace polly {
template <typename Tp>
class optional;

namespace optional_internal {
// THis class template manages constructioin/destruction/assignment of
// the contained value for a polly::optional.
template <typename Tp>
class OptionalDataBase {
public:
  using StoreType = typename std::remove_const<Tp>::type;

  OptionalDataBase() = default;
  ~OptionalDataBase() = default;

  template<typename... Args>
  constexpr explicit OptionalDataBase(in_place_t tag, Args&&... args)
      :  payload_(tag, std::forward<Args>(args)...), engaged_(true) {}

  template<typename Up, typename... Args>
  constexpr explicit OptionalDataBase(
      std::initializer_list<Up> il, Args&&... args)
      :  payload_(il, std::forward<Args>(args)...), engaged_(true) {}

  // Trivially copy/move constructor
  OptionalDataBase(const OptionalDataBase&) = default;
  OptionalDataBase(OptionalDataBase&&) = default;
  OptionalDataBase& operator=(const OptionalDataBase&) = default;
  OptionalDataBase& operator=(OptionalDataBase&&) = default;

  // Non-trivially copy/move constructor
  OptionalDataBase(bool, const OptionalDataBase& other) {
    if (other.engaged_)
      Construct(other.payload_.value_);
  }

  OptionalDataBase(bool, OptionalDataBase&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value) {
    if (other.engaged_)
      Construct(std::move(other.payload_.value_));
  }

  void CopyAssign(const OptionalDataBase& other) {
    if (engaged_ && other.engaged_) {
      payload_.value_ = other.payload_.value_;
    } else {
      if (other.engaged_) {
        Construct(other.payload_.value_);
      } else {
        Reset();
      }
    }
  }

  void MoveAssigin(OptionalDataBase&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    if (engaged_ && other.engaged_) {
      payload_.value_ = std::move(other.payload_.value_);
    } else {
      if (other.engaged_) {
        Construct(std::move(other.payload_.value_));
      } else {
        Reset();
      }
    }
  }

  template<typename... Args>
  void Construct(Args&&... args)
      noexcept(std::is_nothrow_constructible<StoreType, Args...>::value) {
    ::new(std::addressof(payload_.empty_)) StoreType(std::forward<Args>(args)...);
    this->engaged_ = true;
  }

  void Destroy() noexcept {
    payload_.value_.~StoreType();
    engaged_ = false;
  }

  Tp& Get() noexcept { return payload_.value_; }
  constexpr const Tp& Get() const noexcept { return payload_.value_; }
  constexpr bool IsEngaged() const noexcept { return engaged_; }

  void Reset() noexcept {
    if (engaged_)
      Destroy();
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

  Storage<StoreType> payload_;
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
> class OptionalData;

// optional_data with trivial destroy, copy and move.
template <typename Tp>
class OptionalData<Tp, true, true, true>
    : public OptionalDataBase<Tp> {
public:
  using OptionalDataBase<Tp>::OptionalDataBase;

  OptionalData() = default;
};

// optional_data with non-trivial copy construction and assignment.
template <typename Tp>
class OptionalData<Tp, true, false, true>
    : public OptionalDataBase<Tp> {
public:
  using OptionalDataBase<Tp>::OptionalDataBase;

  OptionalData() = default;
  ~OptionalData() = default;

  OptionalData(const OptionalData& other) = default;
  OptionalData(OptionalData&&) = default;
  OptionalData& operator=(OptionalData&&) = default;

  OptionalData& operator=(const OptionalData& other) {
    this->CopyAssign(other);
    return *this;
  }
};

// optional_data with non-trivial move construction and assignment.
template <typename Tp>
class OptionalData<Tp, true, true, false>
    : public OptionalDataBase<Tp> {
public:
  using OptionalDataBase<Tp>::OptionalDataBase;

  OptionalData() = default;
  ~OptionalData() = default;
  OptionalData(const OptionalData&) = default;
  OptionalData& operator=(const OptionalData&) = default;
  OptionalData(OptionalData&& other) = default;

  OptionalData& operator=(OptionalData&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    this->MoveAssigin(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial copy/move construction and assignment.
template <typename Tp>
class OptionalData<Tp, true, false, false>
    : public OptionalDataBase<Tp> {
public:
  using OptionalDataBase<Tp>::OptionalDataBase;

  OptionalData() = default;
  ~OptionalData() = default;
  OptionalData(const OptionalData& other) = default;
  OptionalData(OptionalData&& other) = default;

  OptionalData& operator=(const OptionalData& other) {
    this->CopyAssign(other);
    return *this;
  }

  OptionalData& operator=(OptionalData&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value &&
               std::is_nothrow_move_assignable<Tp>::value) {
    this->MoveAssigin(std::move(other));
    return *this;
  }
};

// optional_data with non-trivial destroy
template <typename Tp, bool Copy, bool Move>
class OptionalData<Tp, false, Copy, Move>
    : public OptionalData<Tp, true, Copy, Move> {
public:
  using OptionalData<Tp, true, false, false>::OptionalData;

  OptionalData() = default;
  OptionalData(const OptionalData&) = default;
  OptionalData(OptionalData&&) = default;
  OptionalData& operator=(OptionalData&&) = default;
  OptionalData& operator=(const OptionalData&) = default;

  ~OptionalData() { this->Reset(); }
};

// Common member functions for optional_base
template <typename Tp, typename Dp>
class OptionalBaseImpl {
protected:
  using StoreType = typename std::remove_const<Tp>::type;

  template <typename... Args>
  void Construct(Args&&... args)
      noexcept(std::is_nothrow_constructible<StoreType, Args...>::value) {
    static_cast<Dp*>(this)->data_.Construct(std::forward<Args>(args)...);
  }

  void Destruct() noexcept {
    static_cast<Dp*>(this)->data_.Destroy();
  }

  void Reset() noexcept {
    static_cast<Dp*>(this)->data_.Reset();
  }

  constexpr bool IsEngaged() const noexcept {
    return static_cast<const Dp*>(this)->data_.IsEngaged();
  }

  Tp& Get() & noexcept {
    return static_cast<Dp*>(this)->data_.Get();
  }

  constexpr const Tp& Get() const & noexcept {
    return static_cast<const Dp*>(this)->data_.Get();
  }
};

template <
    typename Tp,
    bool = is_trivially_copy_constructible<Tp>::value,
    bool = is_trivially_move_constructible<Tp>::value>
class OptionalBase;

template <typename Tp>
class OptionalBase<Tp, false, false>
    : public OptionalBaseImpl<Tp, OptionalBase<Tp>> {
public:
  constexpr OptionalBase() = default;

  template <typename... Args>
  constexpr explicit OptionalBase(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit OptionalBase(
      in_place_t, std::initializer_list<Up> il, Args&&... args)
      : data_(in_place, il, std::forward<Args>(args)...) {}

  constexpr OptionalBase(const OptionalBase& other)
      : data_(other.data_.IsEngaged(), other.data_) {}

  constexpr OptionalBase(OptionalBase&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value)
      : data_(other.data_.IsEngaged(), std::move(other.data_)) {}

  OptionalBase& operator=(const OptionalBase&) = default;
  OptionalBase& operator=(OptionalBase&&) = default;

  OptionalData<Tp> data_;
};

template <typename Tp>
class OptionalBase<Tp, false, true>
    : public OptionalBaseImpl<Tp, OptionalBase<Tp>> {
public:
  constexpr OptionalBase() = default;

  template <typename... Args>
  constexpr explicit OptionalBase(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit OptionalBase(
      in_place_t, std::initializer_list<Up> il, Args&&... args)
      : data_(in_place, il, std::forward<Args>(args)...) {}

  constexpr OptionalBase(const OptionalBase& other)
      : data_(other.data_.IsEngaged(), other.data_) {}

  constexpr OptionalBase(OptionalBase&& other) = default;

  OptionalBase& operator=(const OptionalBase&) = default;
  OptionalBase& operator=(OptionalBase&&) = default;

  OptionalData<Tp> data_;
};

template <typename Tp>
class OptionalBase<Tp, true, false>
    : public OptionalBaseImpl<Tp, OptionalBase<Tp>> {
public:
  constexpr OptionalBase() = default;

  template <typename... Args>
  constexpr explicit OptionalBase(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit OptionalBase(
      in_place_t tag, std::initializer_list<Up> il, Args&&... args)
      : data_(tag, il, std::forward<Args>(args)...) {}

  constexpr OptionalBase(const OptionalBase& other) = default;

  constexpr OptionalBase(OptionalBase&& other)
      noexcept(std::is_nothrow_move_constructible<Tp>::value)
      : data_(other.data_.IsEngaged(), std::move(other.data_)) {}

  OptionalBase& operator=(const OptionalBase&) = default;
  OptionalBase& operator=(OptionalBase&&) = default;

  OptionalData<Tp> data_;
};

template <typename Tp>
class OptionalBase<Tp, true, true>
    : public OptionalBaseImpl<Tp, OptionalBase<Tp>> {
public:
  constexpr OptionalBase() = default;

  template <typename... Args>
  constexpr explicit OptionalBase(in_place_t, Args&&... args)
      : data_(in_place, std::forward<Args>(args)...) {}

  template <typename Up, typename... Args>
  constexpr explicit OptionalBase(
      in_place_t, std::initializer_list<Up> il, Args&&... args)
      : data_(in_place, il, std::forward<Args>(args)...) {}

  constexpr OptionalBase(const OptionalBase& other) = default;
  constexpr OptionalBase(OptionalBase&& other) = default;

  OptionalBase& operator=(const OptionalBase&) = default;
  OptionalBase& operator=(OptionalBase&&) = default;

  OptionalData<Tp> data_;
};

template <typename Tp, typename Up>
using ConvertsFromOptional =
    std::integral_constant<
        bool,
        std::is_constructible<Tp, const optional<Up>&>::value  ||
        std::is_constructible<Tp, optional<Up>&>::value        ||
        std::is_constructible<Tp, const optional<Up>&&>::value ||
        std::is_constructible<Tp, optional<Up>&&>::value       ||
        std::is_convertible<const optional<Up>&, Tp>::value    ||
        std::is_convertible<optional<Up>&, Tp>::value          ||
        std::is_convertible<const optional<Up>&&, Tp>::value   ||
        std::is_convertible<optional<Up>&&, Tp>::value>;

template <typename Tp, typename Up>
using AssignsFromOptional =
    std::integral_constant<
        bool,
        std::is_assignable<Tp&, const optional<Up>&>::value    ||
        std::is_assignable<Tp&, optional<Up>&>::value          ||
        std::is_assignable<Tp&, const optional<Up>&&>::value   ||
        std::is_assignable<Tp&, optional<Up>&&>::value>;

template <typename Tp, typename Tag = void>
using OptionalEnableCopyMove =
    enable_copy_move<
        std::is_copy_constructible<Tp>::value,
        std::is_copy_constructible<Tp>::value &&
        std::is_copy_assignable<Tp>::value,
        std::is_move_constructible<Tp>::value,
        std::is_move_constructible<Tp>::value &&
        std::is_move_assignable<Tp>::value,
        Tag>;

template <typename Tp, typename Optioinal>
using NotSelf = negation<std::is_same<Optioinal, remove_cvref_t<Tp>>>;

template <typename Tp>
using NotInPlaceTag = negation<std::is_same<in_place_t, remove_cvref_t<Tp>>>;

// Helper function for checking whether an expression is convertible to bool.
bool ConvertibleToBool(bool);

#define POLLY_OPTIONAL_CONVERTIBLE_BOOL(exp)  \
  decltype(::polly::optional_internal::ConvertibleToBool(exp))

template <typename Tp, typename = size_t>
struct OptionalHashBase {
  OptionalHashBase() = delete;
  OptionalHashBase(const OptionalHashBase&) = delete;
  OptionalHashBase(OptionalHashBase&&) = delete;
  OptionalHashBase& operator=(const OptionalHashBase&) = delete;
  OptionalHashBase& operator=(OptionalHashBase&&) = delete;
};

#define POLLY_OPTION_HASH_FN                        \
  std::hash<typename std::remove_const<Tp>::type>{} \
      (std::declval<typename std::remove_const<Tp>::type>())

template <typename Tp>
struct OptionalHashBase<Tp, decltype(POLLY_OPTION_HASH_FN)> {
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