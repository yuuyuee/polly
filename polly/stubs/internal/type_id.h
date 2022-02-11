#pragma once

#include "stubs/internal/config.h"

namespace polly {
namespace type_id_internal {
template <typename Tp>
struct TypeTag {
  static constexpr const char kId = 0;
};

template <typename Tp>
constexpr const char TypeTag<Tp>::kId;

using TypeIdType = const void*;

template <typename Tp>
constexpr inline TypeIdType TypeId() {
  return &TypeTag<Tp>::kId;
}

} // namespace type_id_internal
} // namespace polly
