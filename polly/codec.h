// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_CODEC_H_
#define OAK_COMMON_CODEC_H_

#include <stdint.h>
#include <stddef.h>

namespace oak {
#define OAK_INT(bit) int ## bit ## _t
#define OAK_UINT(bit) uint ## bit ## _t

#define OAK_INT_ACCESS_FN(bit)                                  \
OAK_INT(bit) GetInt ## bit(void* buffer) {                      \
  return *reinterpret_cast<const OAK_INT(bit)*>(buffer);        \
}                                                               \
                                                                \
OAK_UINT(bit) GetUInt ## bit(void* buffer) {                    \
  return *reinterpret_cast<const OAK_UINT(bit)*>(buffer);       \
}                                                               \
                                                                \
void SetInt ## bit(void* buffer, OAK_INT(bit) value) {          \
  *reinterpret_cast<OAK_INT(bit)*>(buffer) = value;             \
}                                                               \
                                                                \
void SetUInt ## bit(void* buffer, OAK_UINT(bit) value) {        \
  *reinterpret_cast<OAK_UINT(bit)*>(buffer) = value;            \
}

OAK_INT_ACCESS_FN(8)
OAK_INT_ACCESS_FN(16)
OAK_INT_ACCESS_FN(32)
OAK_INT_ACCESS_FN(64)

#define OAK_ZIGZAG_FN(bit)                                      \
inline OAK_UINT(bit)                                            \
    ZigZagEnode ## bit(OAK_INT(bit) value) {                    \
  return (value << 1) ^ (value >> (bit - 1));                   \
}                                                               \
                                                                \
inline OAK_INT(bit)                                             \
    ZigZagDecode ## bit(OAK_UINT(bit) value) {                  \
  return (value >> 1) ^ -static_cast<OAK_INT(bit)>(value & 1);  \
}

OAK_ZIGZAG_FN(8)
OAK_ZIGZAG_FN(16)
OAK_ZIGZAG_FN(32)
OAK_ZIGZAG_FN(64)

inline const void* GetVarint32(const void* buffer, uint32_t* value) {
  const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buffer);
  *value = ptr[0] & 0x7F;
  if (ptr[0] & 0x80) {
    *value |= (ptr[1] & 0x7F) << 7;
    if (ptr[1] & 0x80) {
      *value |= (ptr[2] & 0x7F) << 14;
      if (ptr[2] & 0x80) {
        *value |= (ptr[3] & 0x7F) << 21;
        if (ptr[3] & 0x80) {
          *value |= (ptr[4] & 0x7F) << 28;
          return ptr + 5;
        } else {
          return ptr + 4;
        }
      } else {
        return ptr + 3;
      }
    } else {
      return ptr + 2;
    }
  } else {
    return ptr + 1;
  }
}

inline void* SetVarint32(void* buffer, uint32_t value) {
  uint8_t* ptr = reinterpret_cast<uint8_t*>(buffer);
  ptr[0] = static_cast<uint8_t>(value | 0x80);
  value >>= 7;
  if (value > 0) {
    ptr[1] = static_cast<uint8_t>(value | 0x80);
    value >>= 7;
    if (value > 0) {
      ptr[2] = static_cast<uint8_t>(value | 0x80);
      value >>= 7;
      if (value > 0) {
        ptr[3] = static_cast<uint8_t>(value | 0x80);
        value >>= 7;
        if (value > 0) {
          ptr[4] = static_cast<uint8_t>(value | 0x7F);
          return ptr + 5;
        } else {
          ptr[3] &= 0x7F;
          return ptr + 4;
        }
      } else {
        ptr[2] &= 0x7F;
        return ptr + 3;
      }
    } else {
      ptr[1] &= 0x7F;
      return ptr + 2;
    }
  } else {
    ptr[0] &= 0x7F;
    return ptr + 1;
  }
}

}  // namespace oak

#endif  // OAK_COMMON_CODEC_H_
