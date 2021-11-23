#pragma once

#include <cstddef>
#include <cstring>

namespace polly {
// Implementation of Murmur hash for 64-bit size_t.
size_t MurMurHash64(const void* ptr, size_t len, size_t seed = 0xC70F6907UL);

// Implementation of FNV-1a hash for 64-bit size_t.
size_t FNVHash64(const void* ptr, size_t len, size_t seed = 0x811C9DC5UL);

}  // namespace std