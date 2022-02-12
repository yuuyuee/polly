#pragma once

#include "stubs/internal/config.h"

#if defined(POLLY_HAVE_RTTI)
#include <typeinfo>
#endif
#include "stubs/type_traits.h"
#include "stubs/utility.h"

namespace polly {
namespace any_internal {
// Avoid dynamic allocations for small objects, but such an optimization
// may only be applied to types for which std::is_nothrow_move_constructible
// returns true.
union Storage {
  // uses small-object optimization
  alignas(sizeof(void*)) char buffer[sizeof(void*)];
  // creates object on the heap
  void* ptr;
};

template <typename Tp>
struct is_small_obj
    : public std::integral_constant<
        bool,
        sizeof(Tp) <= sizeof(void*) &&
        alignof(Tp) <= alignof(void*) &&
        std::is_nothrow_move_constructible<Tp>::value> {};

template <typename Tp, bool = is_small_obj<Tp>::value>
struct Operator {
  // Constructors and destructor
  template <typename Up>
  static void Construct(Storage& storage, Up&& value) {
    storage.ptr = new Tp(std::forward<Up>(value));
  }

  template <typename... Args>
  static void Construct(Storage& storage, Args&&... args) {
    storage.ptr = new Tp(std::forward<Args>(args)...);
  }

  static void CopyConstruct(Storage& storage, const Tp* ptr) {
    storage.ptr = new Tp(*ptr);
  }

  static void MoveConstruct(Storage& storage, Tp* ptr) {
    storage.ptr = ptr;
  }

  static void Destruct(Storage& storage) {
    delete storage.ptr;
  }

  // Accessers
  Tp* get(Storage& storage) {
    return reinterpret_cast<Tp*>(storage.ptr);
  }

  const Tp* get(const Storage& storage) {
    return reinterpret_cast<const Tp*>(storage.ptr);
  }

  // type_info
  const std::type_info& Type() {
    return typeid(Tp);
  }
};

template <typename Tp>
struct Operator<Tp, true> {
  template <typename Up>
  static void construct(Storage& storage, Up&& value) {
    ::new(storage.buffer) Tp(std::forward<Up>(value));
  }

  template <typename... Args>
  static void construct(Storage& storage, Args&&... args) {
    ::new(storage.buffer) Tp(std::forward<Args>(args)...);
  }

  static void CopyConstruct(Storage& storage, const Tp* ptr) {
    ::new(storage.buffer) Tp(*ptr);
  }

  static void MoveConstruct(Storage& storage, Tp* ptr) {
    ::new(storage.buffer) Tp(std::move(*ptr));
    ptr->~Tp();
  }

  static void Destruct(Storage& storage) {
    get(storage)->~Tp();
  }

  Tp* get(Storage& storage) {
    return reinterpret_cast<Tp*>(storage.buffer);
  }

  const Tp* get(const Storage& storage) {
    return reinterpret_cast<const Tp*>(storage.buffer);
  }

  const std::type_info& Type() {
    return typeid(Tp);
  }
};


} // namespace any_internal
} // namespace polly
