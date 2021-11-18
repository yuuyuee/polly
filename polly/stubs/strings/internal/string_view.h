#pragma once

#include <cstring>
#include <string>
#include <iterator>
#include <limits>
#include <algorithm>

#include "stubs/base/const.h"
#include "stubs/base/macros.h"
#include "stubs/base/exception.h"

namespace polly {
template<typename CharT, typename Traits = std::char_traits<CharT>>
class basic_string_view {
public:
  // Member types
  using traits_type = Traits;
  using value_type = CharT;
  using pointer = CharT*;
  using const_pointer = const CharT*;
  using reference = CharT&;
  using const_reference = const CharT&;
  using const_iterator = const CharT*;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  static constexpr size_type npos = static_cast<size_type>(-1);

  constexpr basic_string_view() noexcept: ptr_(nullptr), size_(0) {}
  constexpr explicit basic_string_view(const char* s) noexcept
      : ptr_(s), len_(s ? ConstStrlen(s) : 0) {}
  constexpr basic_string_view(const char* s, size_t n) noexcept
      : ptr_(s), len_(n) {}

  // Iterator
  constexpr const_iterator begin() const noexcept { return ptr_; }
  constexpr const_iterator end() const noexcept { return ptr_ + len_; }
  constexpr const_iterator cbegin() const noexcept { return begin(); }
  constexpr const_iterator cend() const noexcept { return end(); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // Element access
  constexpr const_reference operator[](size_type i) const {
    return POLLY_CONST_ASSERT(i < size()), ptr_[i];
  }

  constexpr const_reference at(size_type i) const {
    return POLLY_EXPECT_TRUE(i < size())
        ? ptr_[i] : (polly::ThrowStdOutOfRange("polly::string_view::at"), ptr_[i]);
  }
  constexpr const_reference front() const {
    return POLLY_CONST_ASSERT(!empty()), ptr_[0];
  }
  constexpr const_reference back() const {
    return POLLY_CONST_ASSERT(!empty()), ptr_[size() - 1];
  }
  constexpr const_pointer data() const noexcept { return ptr_; }

  // Capacity
  constexpr size_type size() const noexcept { return len_; }
  constexpr size_type length() const noexcept { return size(); }
  constexpr size_type max_size() const noexcept { return kMaxSize; }
  constexpr bool empty() const noexcept { return len_ == 0; }

  // Modifiers
  void remove_prefix(size_type n) {
    POLLY_CONST_ASSERT(n <= len_);
    ptr_ += n;
    len_ -= n;
  }

  void remove_suffix(size_type n) {
    POLLY_CONST_ASSERT(n <= len_);
    len_ -= n;
  }

  void swap(string_view& other) noexcept {
    auto tmp = *this;
    *this = other;
    other = tmp;
  }

  // Operations
  size_type copy(char* buf, size_type n, size_type pos = 0) const {
    if (POLLY_EXPECT_FALSE(pos > len_))
      polly::ThrowStdOutOfRange("polly::string_view::copy");

    n = std::min(len_ - pos, n);
    if (n > 0)
      traits_type::copy(buf, ptr_ + pos, n);
    return n;
  }

  constexpr string_view substr(size_type pos = 0, size_type n = npos) const {
    return POLLY_EXPECT_FALSE(pos > len_)
        ? (polly::ThrowStdOutOfRange("polly::string_view::substr"), string_view())
        : string_view(ptr_ + pos, std::min(n, len_ - pos));
  }

  constexpr int compare(string_view other) const noexcept {
    return CompareHelper(
        len_, other.len_,
        std::min(len_, other.len_) == 0
          ? 0
          : memcmp(ptr_, other.ptr_, std::min(len_, other.len_)));
  }

  constexpr int compare(size_type pos, size_type n, string_view other) const {
    return substr(pos, n).compare(other);
  }

  constexpr int compare(const char* s) const { return compare(string_view(s)); }
  constexpr int compare(size_type pos, size_type n, const char* s) const {
    return substr(pos, n).compare(string_view(s));
  }

  size_type find(string_view s, size_type pos = 0) const noexcept;
  size_type find(char c, size_type pos = 0) const noexcept;
  size_type find(const char* s, size_type pos, size_type count) const {
    return find(string_view(s, count), pos);
  }
  size_type find(const char* s, size_type pos = 0) const {
    return find(string_view(s), pos);
  }

  size_type rfind(string_view s, size_type pos = npos) const noexcept;
  size_type rfind(char c, size_type pos = npos) const noexcept;
  size_type rfind(const char* s, size_type pos, size_type count) const {
    return rfind(string_view(s, count), pos);
  }
  size_type rfind(const char* s, size_type pos = npos) const {
    return rfind(string_view(s), pos);
  }

  size_type find_first_of(string_view s, size_type pos = 0) const noexcept;
  size_type find_first_of(char c, size_type pos = 0) const noexcept {
    return find(c, pos);
  }

  size_type find_first_of(const char* s, size_type pos,
                                    size_type count) const {
    return find_first_of(string_view(s, count), pos);
  }

  size_type find_first_of(const char* s, size_type pos = 0) const {
    return find_first_of(string_view(s), pos);
  }

  size_type find_last_of(string_view s, size_type pos = npos) const noexcept;
  size_type find_last_of(char c, size_type pos = npos) const noexcept {
    return rfind(c, pos);
  }
  size_type find_last_of(const char* s, size_type pos, size_type count) const {
    return find_last_of(string_view(s, count), pos);
  }
  size_type find_last_of(const char* s, size_type pos = npos) const {
    return find_last_of(string_view(s), pos);
  }


  size_type find_first_not_of(string_view s, size_type pos = 0) const noexcept;
  size_type find_first_not_of(char c, size_type pos = 0) const noexcept;
  size_type find_first_not_of(const char* s, size_type pos,
                              size_type count) const {
    return find_first_not_of(string_view(s, count), pos);
  }
  size_type find_first_not_of(const char* s, size_type pos = 0) const {
    return find_first_not_of(string_view(s), pos);
  }

  size_type find_last_not_of(string_view s,
                             size_type pos = npos) const noexcept;
  size_type find_last_not_of(char c, size_type pos = npos) const noexcept;
  size_type find_last_not_of(const char* s, size_type pos,
                             size_type count) const {
    return find_last_not_of(string_view(s, count), pos);
  }
  size_type find_last_not_of(const char* s, size_type pos = npos) const {
    return find_last_not_of(string_view(s), pos);
  }


  template <typename Alloc>
  explicit operator std::basic_string<CharT, traits_type, Alloc>() const {
    if (!data()) return {};
    return std::basic_string<CharT, traits_type, Alloc>(data(), size());
  }

private:
  static constexpr size_type kMaxSize = std::numeric_limits<difference_type>::max();

  static constexpr int CompareHelper(size_type a, size_type b, int r) {
    return r == 0 ? static_cast<int>(a > b) - static_cast<int>(a < b) : (r < 0 ? -1 : 1);
  }

private:

  pointer ptr_;
  size_type len_;
};

using string_view = basic_string_view<char>;

constexpr bool operator==(string_view a, string_view b) noexcept {
  return a.compare(b) == 0;
}

constexpr bool operator!=(string_view a, string_view b) noexcept {
  return !(a == b);
}

constexpr bool operator<(string_view a, string_view b) noexcept {
  return a.compare(b) < 0;
}

constexpr bool operator>(string_view a, string_view b) noexcept {
  return b < a;
}

constexpr bool operator<=(string_view a, string_view b) noexcept {
  return !(b < a);
}

constexpr bool operator>=(string_view a, string_view b) noexcept {
  return !(a < b);
}

} // namespace polly