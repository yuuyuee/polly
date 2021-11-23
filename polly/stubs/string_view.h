#pragma once

#include "stubs/base/config.h"

#if defined(POLLY_HAVE_STD_STRING_VIEW)
#include <string_view>

namespace polly {
using std::string_view;
} // namespace polly
#else // POLLY_HAVE_STD_STRING_VIEW
#include <cstring>
#include <string>
#include <iterator>
#include <limits>
#include <algorithm>
#include <ostream>

#include "stubs/base/attributes.h"
#include "stubs/base/check.h"
#include "stubs/base/const.h"
#include "stubs/base/exception.h"

template<> struct std::char_traits<char>;

namespace polly {
template<typename Char, typename Traits = std::char_traits<Char>>
class basic_string_view;

using string_view = basic_string_view<char, std::char_traits<char>>;

template<typename Char, typename Traits>
class basic_string_view {
public:
  // Member types
  using traits_type = Traits;
  using value_type = Char;
  using pointer = Char*;
  using const_pointer = const Char*;
  using reference = Char&;
  using const_reference = const Char&;
  using const_iterator = const Char*;
  using iterator = const_iterator;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  using reverse_iterator = const_reverse_iterator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  static const size_type npos;

  // Null `basic_string_view` constructor.
  constexpr basic_string_view() noexcept: ptr_(nullptr), len_(0) {}

  // Implicit constructor of a `basic_string_view' from NUL-terminated `str'.
  constexpr basic_string_view(const_pointer s) // NOLINT
      : ptr_(s), len_(s ? traits_type::length(s) : 0) {}

  // Implicit constructor of a `basic_string_view' from a `const char*' and `n'.
  constexpr basic_string_view(const Char* s, size_t n)
      : ptr_(s), len_(n) {}

  // Implicit constructors of a `basic_string_view' from `std::basic_string'.
  template<typename Alloc>
  basic_string_view(const std::basic_string<Char, traits_type, Alloc>& s) noexcept
      : basic_string_view(s.data(), s.size()) {}

  // constexpr basic_string_view(const basic_string_view&) noexcept = default;
  // basic_string_view& operator=(const basic_string_view&) noexcept = default;

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

  // Return a const reference to the character at specified location pos.
  // No bounds checking is performed, the behavior is undefined if pos >= size().
  constexpr const_reference operator[](size_type pos) const {
    return POLLY_CONST_ASSERT(pos < size()), ptr_[pos];
  }

  // Resturn a const reference to the character at specified location pos.
  // Bounds checking is performed, exception of type `std::out_of_range'
  // will be thrown on invalid access.
  constexpr const_reference at(size_type pos) const {
    return POLLY_EXPECT_TRUE(pos < size())
        ? ptr_[pos] : (polly::ThrowStdOutOfRange("polly::string_view::at"), ptr_[pos]);
  }

  // Return reference to the first character in the view. the behavior is
  // undefined if empty() == true.
  constexpr const_reference front() const {
    return POLLY_CONST_ASSERT(!empty()), ptr_[0];
  }

  // Return reference to the last character in the view. the behavior is
  // undefined if empty() == true.
  constexpr const_reference back() const {
    return POLLY_CONST_ASSERT(!empty()), ptr_[size() - 1];
  }

  // Return a pointer to the underlying character array.
  constexpr const_pointer data() const noexcept { return ptr_; }

  // Capacity

  // Return the number of character elements in the view.
  constexpr size_type size() const noexcept { return len_; }
  constexpr size_type length() const noexcept { return size(); }

  // Return maximum number of characters.
  constexpr size_type max_size() const noexcept { return kMaxSize; }

  // Return true if the view is empty, false otherwise.
  constexpr bool empty() const noexcept { return len_ == 0; }

  // Modifiers

  // Moves the start of the view forward by n characters. the behavior
  // is undefined if n > size().
  void remove_prefix(size_type n) {
    POLLY_CONST_ASSERT(n <= len_);
    ptr_ += n;
    len_ -= n;
  }

  // Moves the end of the view back by n characters. the behavior is
  // undefined if n > size().
  void remove_suffix(size_type n) {
    POLLY_CONST_ASSERT(n <= len_);
    len_ -= n;
  }

  // Exchanges the view with that of other.
  void swap(basic_string_view& other) noexcept {
    auto tmp = *this;
    *this = other;
    other = tmp;
  }

  // Operations

  // Copies the substring[pos, pos + rcount) to the character array pointed
  // to by dest, where rcount is the samller of count and size() - pos.
  size_type copy(Char* buf, size_type n, size_type pos = 0) const {
    if (POLLY_EXPECT_FALSE(pos > len_))
      polly::ThrowStdOutOfRange("polly::string_view::copy");

    n = std::min(len_ - pos, n);
    if (n > 0)
      traits_type::copy(buf, ptr_ + pos, n);
    return n;
  }

  // Returns a view of the substring[pos, pos + rcount), where rcount is the
  // smaller of count and size() - pos.
  constexpr basic_string_view substr(size_type pos = 0, size_type n = npos) const {
    return POLLY_EXPECT_FALSE(pos > len_)
        ? (polly::ThrowStdOutOfRange("polly::string_view::substr"), basic_string_view())
        : basic_string_view(ptr_ + pos, polly::ConstMin(n, len_ - pos));
  }

  // Compares tow character sequences, negative value if this view is less
  // than the other character sequence, zero if the both character sequences
  // are equal, positive value if this view is greater than the other
  // character sequence.
  constexpr int compare(basic_string_view other) const noexcept {
    return CompareHelper(len_, other.len_,
        std::min(len_, other.len_) == 0 ? 0
          : traits_type::compare(ptr_, other.ptr_, std::min(len_, other.len_)));
  }

  constexpr int compare(
      size_type pos1, size_type count1, basic_string_view other) const {
    return substr(pos1, count1).compare(other);
  }

  constexpr int compare(
      size_type pos1, size_type count1,
      basic_string_view other, size_type pos2, size_type count2) const {
    return substr(pos1, count1).compare(other.substr(pos2, count2));
  }

  constexpr int compare(const Char* s) const {
    return compare(basic_string_view(s));
  }

  constexpr int compare(
      size_type pos1, size_type count1, const Char* s) const {
    return substr(pos1, count1).compare(basic_string_view(s));
  }

  constexpr int compare(
      size_type pos1, size_type count1,
      const Char* s, size_type pos2, size_type count2) const {
    return substr(pos1, count1).compare(basic_string_view(s).substr(pos2, count2));
  }

  // Checks if the string view begins with the given prefix. true if
  // string view begin with the provided prefix, false otherwise.
  constexpr bool starts_with(basic_string_view other) const noexcept {
    return size() >= other.size() && !compare(0, other.size(), other);
  }

  constexpr bool starts_with(Char c) const noexcept {
    return !empty() && traits_type::eq(front(), c);
  }

  constexpr bool starts_with(const Char* s) const noexcept {
    return starts_with(basic_string_view(s));
  }

  constexpr bool starts_with(const Char* s, size_type n) const noexcept {
    return starts_with(basic_string_view(s, n));
  }

  // Checks if the string view ends with the given suffix. true if
  // string view ends with the provided prefix, false otherwise.
  constexpr bool ends_with(basic_string_view other) const noexcept {
    return size() >= other.size() &&
        !compare(size() - other.size(), npos, other);
  }

  constexpr bool ends_with(Char c) const noexcept {
    return !empty() && traits_type::eq(back(), c);
  }

  constexpr bool ends_with(const Char* s) const noexcept {
    return ends_with(basic_string_view(s));
  }

  constexpr bool ends_with(const Char* s, size_type n) const noexcept {
    return ends_with(basic_string_view(s, n));
  }

  // basic_string_view::find

  // Find the first substring equal to the given character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the first character of the found substring, or `npos'
  // if no such substring is found.
  size_type find(basic_string_view v, size_type pos = 0) const noexcept;
  size_type find(Char c, size_type pos = 0) const noexcept;
  size_type find(const Char* s, size_type pos, size_type count) const {
    return find(basic_string_view(s, count), pos);
  }
  size_type find(const Char* s, size_type pos = 0) const {
    return find(basic_string_view(s), pos);
  }

  // Find the last substring equal to the given character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the first character of the found substring, or `npos'
  // if no such substring is found.
  size_type rfind(basic_string_view v, size_type pos = npos) const noexcept;
  size_type rfind(Char c, size_type pos = npos) const noexcept;
  size_type rfind(const Char* s, size_type pos, size_type count) const {
    return rfind(basic_string_view(s, count), pos);
  }
  size_type rfind(const Char* s, size_type pos = npos) const {
    return rfind(basic_string_view(s), pos);
  }

  // Find the first character equal to any of the character in the given
  // character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the first occurence of any character of the substring,
  // or `npos' is no such character is found.
  size_type find_first_of(basic_string_view s, size_type pos = 0) const noexcept;
  size_type find_first_of(Char c, size_type pos = 0) const noexcept {
    return find(c, pos);
  }
  size_type find_first_of(const Char* s, size_type pos,
                                    size_type count) const {
    return find_first_of(string_view(s, count), pos);
  }
  size_type find_first_of(const Char* s, size_type pos = 0) const {
    return find_first_of(basic_string_view(s), pos);
  }

  // Find the last character equal to any of the character in the given
  // character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the last occurence of any character of the substring,
  // or `npos' is no such character is found.
  size_type find_last_of(basic_string_view s, size_type pos = npos) const noexcept;
  size_type find_last_of(Char c, size_type pos = npos) const noexcept {
    return rfind(c, pos);
  }
  size_type find_last_of(const Char* s, size_type pos, size_type count) const {
    return find_last_of(string_view(s, count), pos);
  }
  size_type find_last_of(const Char* s, size_type pos = npos) const {
    return find_last_of(string_view(s), pos);
  }

  // Find the first character not equal to any of the character in the given
  // character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the first character not equal to any character of
  // the substring, or `npos' is no such character is found.
  size_type find_first_not_of(basic_string_view s, size_type pos = 0) const noexcept;
  size_type find_first_not_of(Char c, size_type pos = 0) const noexcept;
  size_type find_first_not_of(const Char* s, size_type pos,
                              size_type count) const {
    return find_first_not_of(basic_string_view(s, count), pos);
  }
  size_type find_first_not_of(const Char* s, size_type pos = 0) const {
    return find_first_not_of(basic_string_view(s), pos);
  }

  // Find the last character not equal to any of the character in the given
  // character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the last character not equal to any character of
  // the substring, or `npos' is no such character is found.
  size_type find_last_not_of(basic_string_view s,
                             size_type pos = npos) const noexcept;
  size_type find_last_not_of(Char c, size_type pos = npos) const noexcept;
  size_type find_last_not_of(const Char* s, size_type pos,
                             size_type count) const {
    return find_last_not_of(string_view(s, count), pos);
  }
  size_type find_last_not_of(const Char* s, size_type pos = npos) const {
    return find_last_not_of(basic_string_view(s), pos);
  }

  // Explicit converts to std::basic_string.
  template <typename Alloc>
  explicit operator std::basic_string<Char, traits_type, Alloc>() const {
    if (!data()) return {};
    return std::basic_string<Char, traits_type, Alloc>(data(), size());
  }

private:
  static constexpr size_type kMaxSize =
      std::numeric_limits<difference_type>::max() / sizeof(Char);

  static constexpr int CompareHelper(size_type a, size_type b, int r) {
    return r == 0 ? static_cast<int>(a > b) - static_cast<int>(a < b)
        : (r < 0 ? -1 : 1);
  }

  const_pointer ptr_;
  size_type len_;
};

template<typename Char, typename Traits>
const typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::npos = static_cast<size_type>(-1);

namespace string_view_internal {
template<typename Char, typename Traits>
const char* FindHelper(
    const Char* haystack, size_t haylen,
    const Char* needle, size_t nlen) {
  if (nlen == 0)
    return haystack;
  if (haylen < nlen)
    return nullptr;

  const Char* match;
  const Char* hayend = haystack + haylen - nlen + 1;
  while ((match = static_cast<const Char*>(
      Traits::find(haystack, hayend - haystack, needle[0])))) {
    if (!Traits::compare(match, needle, nlen))
      return match;
    haystack = match + 1;
  }
  return nullptr;
}
} // namespace string_view_internal

// basic_string_view::find
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find(
    basic_string_view v, size_type pos) const noexcept {
  if (v.len_ == 0)
    return pos <= len_ ? pos : npos;

  if (v.len_ <= len_) {
    for (; pos <= len_ - v.len_; ++pos) {
      if (traits_type::eq(ptr_[pos], v.ptr_[0]) &&
          !traits_type::compare(ptr_ + pos + 1, v.ptr_ + 1, v.len_ - 1)) {
        return pos;
      }
    }
  }
  return npos;
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find(Char c, size_type pos) const noexcept {
  size_type ret = npos;
  if (pos < len_) {
    const Char* p = traits_type::find(ptr_ + pos, len_ - pos, c);
    if (p)
      ret = p - ptr_;
  }
  return ret;
}

// basic_string_view::rfind
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::rfind(
    basic_string_view v, size_type pos) const noexcept {
  if (v.len_ <= len_) {
    pos = std::min(size_type(len_ - v.len_), pos);
    do {
      if (!traits_type::compare(ptr_ + pos, v.ptr_, v.len_))
        return pos;
    } while (pos-- > 0);
  }
  return npos;
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::rfind(Char c, size_type pos) const noexcept {
  if (len_ > 0) {
    for (pos = std::min(len_ - 1, pos) + 1; pos > 0; --pos) {
      if (traits_type::eq(ptr_[pos - 1], c))
        return pos - 1;
    }
  }
  return npos;
}

// basic_string_view::find_first_of
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_first_of(
    basic_string_view v, size_type pos) const noexcept {
  if (v.len_ > 0) {
    for (; pos < len_; ++pos) {
      if (traits_type::find(v.ptr_, v.len_, ptr_[pos]))
        return pos;
    }
  }
  return npos;
}

// basic_string_view::find_last_of
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_last_of(
    basic_string_view v, size_type pos) const noexcept {
  if (len_ > 0 && v.len_ > 0) {
    for (pos = std::min(pos, len_ - 1) + 1; pos > 0; --pos) {
      if (traits_type::find(v.ptr_, v.len_, ptr_[pos - 1]))
        return pos - 1;
    }
  }
  return npos;
}

// basic_string_view::find_first_not_of
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_first_not_of(
    basic_string_view v, size_type pos) const noexcept {
  for (; pos < len_; ++pos) {
    if (!traits_type::find(v.ptr_, v.len_, ptr_[pos]))
      return pos;
  }
  return npos;
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_first_not_of(
    Char c, size_type pos) const noexcept {
  for (; pos < len_; ++pos) {
    if (!traits_type::eq(c, ptr_[pos]))
      return pos;
  }
  return npos;
}

// basic_string_view::find_last_not_of
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_last_not_of(
    basic_string_view v, size_type pos) const noexcept {
  if (v.len_ == 0)
    return len_ != 0 ? std::min(pos, len_ - 1) : npos;
  if (len_ > 0) {
    for (pos = std::min(pos, len_ - 1) + 1; pos > 0; --pos) {
      if (!traits_type::find(v.ptr_, v.len_, ptr_[pos - 1]))
        return pos - 1;
    }
  }
  return npos;
}

template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_last_not_of(
    Char c, size_type pos) const noexcept {
  if (len_ > 0) {
    for (pos = std::min(pos, len_ - 1) + 1; pos > 0; --pos) {
      if (!traits_type::eq(c, ptr_[pos - 1]))
        return pos - 1;
    }
  }
  return npos;
}

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

namespace string_view_internal {
template<typename Char, typename Traits>
void WritePadding(std::basic_ostream<Char, Traits>& o, size_t pad) {
  char fill_buf[32];
  memset(fill_buf, o.fill(), sizeof(fill_buf));
  while (pad) {
    size_t n = std::min(pad, sizeof(fill_buf));
    o.write(fill_buf, n);
    pad -= n;
  }
}
} // namespace string_view_internal

// IO Insertion Operator
template<typename Char, typename Traits>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& o, basic_string_view<Char, Traits> v) {
  std::ostream::sentry sentry(o);
  if (sentry) {
    size_t lpad = 0;
    size_t rpad = 0;
    if (static_cast<size_t>(o.width()) > v.size()) {
      size_t pad = o.width() - v.size();
      if ((o.flags() & o.adjustfield) == o.left) {
        rpad = pad;
      } else {
        lpad = pad;
      }
    }
    if (lpad)
      string_view_internal::WritePadding(o, lpad);
    o.write(v.data(), v.size());
    if (rpad)
      string_view_internal::WritePadding(o, rpad);
    o.width(0);
  }
  return o;
}

} // namespace polly
#endif // POLLY_HAVE_STD_STRING_VIEW