#pragma once

#include "stubs/internal/config.h"

#include <typeinfo>
#include "stubs/type_traits.h"
#include "stubs/utility.h"

namespace polly {
namespace any_internal {
// Avoid dynamic allocations for small objects, but such an optimization
// may only be applied to types for which std::is_nothrow_move_constructible
// returns true.
union Storage {
  constexpr Storage() noexcept: ptr(nullptr) {}

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

struct Args {
  void* obj;
  const std::type_info* typeinfo;
  Storage* storage;
};

enum Ops {
  Get, GetTypeInfo, Copy, Move, Destruct
};

// contained object
template <typename Tp, bool = is_small_obj<Tp>::value>
struct Operators {
  // Constructors and destructor
  template <typename VTp>
  static void Construct(Storage* storage, VTp&& value) {
    storage->ptr = new Tp(std::forward<VTp>(value));
  }

  template <typename... Args>
  static void Construct(Storage* storage, Args&&... args) {
    storage->ptr = new Tp(std::forward<Args>(args)...);
  }

  static void Operator(Ops ops, const Storage* self, Args* args) {
    auto ptr = reinterpret_cast<const Tp*>(self->ptr);
    switch (ops) {
    case Ops::Get:
      args->obj = const_cast<Tp*>(ptr);
      break;
    case Ops::GetTypeInfo:
#ifdef POLLY_HAVE_RTTI
      args->typeinfo = &typeid(Tp);
#endif
      break;
    case Ops::Copy:
      args->storage->ptr = new Tp(*ptr);
      break;
    case Ops::Move:
      args->storage->ptr = const_cast<Tp*>(ptr);
      break;
    case Ops::Destruct:
      delete ptr;
      break;
    }
  }
};

// in-place contained object
template <typename Tp>
struct Operators<Tp, true> {
  template <typename VTp>
  static void Construct(Storage* storage, VTp&& value) {
    ::new(storage->buffer) Tp(std::forward<VTp>(value));
  }

  template <typename... Args>
  static void Construct(Storage* storage, Args&&... args) {
    ::new(storage->buffer) Tp(std::forward<Args>(args)...);
  }

  static void Operator(Ops ops, const Storage* self, Args* args) {
    auto ptr = reinterpret_cast<const Tp*>(self->buffer);
    switch (ops) {
    case Ops::Get:
      args->obj = const_cast<Tp*>(ptr);
      break;
    case Ops::GetTypeInfo:
#ifdef POLLY_HAVE_RTTI
      args->typeinfo = &typeid(Tp);
#endif
      break;
    case Ops::Copy:
      ::new (&args->storage->buffer) Tp(*ptr);
      break;
    case Ops::Move:
      ::new (&args->storage->buffer) Tp(std::move(*const_cast<Tp*>(ptr)));
      ptr->~Tp();
      break;
    case Ops::Destruct:
      ptr->~Tp();
      break;
    }
  }
};

template <typename Tp>
struct is_in_place_type: public std::false_type {};

template <typename Tp>
struct is_in_place_type<in_place_type_t<Tp>>: public std::true_type {};

} // namespace any_internal
} // namespace polly
