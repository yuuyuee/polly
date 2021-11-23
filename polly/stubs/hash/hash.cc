#include "stubs/hash/hash.h"
#include <cstring>

namespace polly {
namespace {
inline size_t UnAlignedLoad(const char* ptr) {
  size_t result;
  memcpy(&result, ptr, sizeof(result));
  return result;
}

// Loads n bytes, where 1 <= n < 8
inline size_t LoadBytes(const char* ptr, size_t len) {
  size_t result = 0;
  for (size_t pos = len; pos > 0; --pos)
    result = (result << 8) + static_cast<unsigned char>(ptr[pos - 1]);
  return result;
}

inline size_t ShiftMix(size_t v) {
  return v ^ (v >> 47);
}

} // anonymous namespace

// Implementation of Murmur hash for 64-bit size_t.
size_t MurMurHash64(const void* ptr, size_t len, size_t seed) {
  static constexpr const size_t kMul = (0xC6A4A793UL << 32) +  0x5bd1e995UL;
  const char* cptr = static_cast<const char*>(ptr);

  // Remove the bytes not divisible by the sizeof(size_t).
  // This allows the main loop to process the data as 64-bit integers.
  const size_t len_aligned = len & ~static_cast<size_t>(0x7U);
  const char* const end = cptr + len_aligned;
  size_t hash = seed ^ (len * kMul);

  for (; cptr != end; cptr += 8) {
    hash ^= ShiftMix(UnAlignedLoad(cptr) * kMul) * kMul;
    hash *= kMul;
  }

  if ((len & 0x7U) != 0) {
    hash ^= LoadBytes(end, len & 0x7U);
    hash *= kMul;
  }

  hash = ShiftMix(hash) * kMul;
  hash = ShiftMix(hash);
  return hash;
}

// Implementation of FNV-1a hash for 64-bit size_t.
size_t FNVHash64(const void* ptr, size_t len, size_t seed) {
  size_t hash = seed;
  const char* cptr = static_cast<const char*>(ptr);
  for (; len > 0; --len) {
    hash ^= static_cast<size_t>(*cptr++);
    hash *= static_cast<size_t>(0x100000001B3UL);
  }
  return hash;
}
} // namespace polly