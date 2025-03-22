// Copyright 2022 The Oak Authors.

#ifndef OAK_COMMON_STRINGPIECE_H_
#define OAK_COMMON_STRINGPIECE_H_

#include <cstring>
#include <string>
#include <iterator>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <algorithm>

#include "oak/common/macros.h"

namespace oak {
// The class template StringPiece describes an object that can refer to
// a constant contiguous sequence of char-like objects with the first element
// of the sequence at position zero. it's the users responsibility to ensure
// that std::StringPieceImpl does not outlive the pointed to character array.
template<typename Char, typename Traits = std::char_traits<Char>>
class StringPieceImpl;

using StringPiece = StringPieceImpl<char, std::char_traits<char>>;

template<typename Char, typename Traits>
class StringPieceImpl {
 public:
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

#if defined(__cpp_inline_variables)
  static inline constexpr size_type npos = static_cast<size_type>(-1);
#else
  static constexpr size_type npos = static_cast<size_type>(-1);
#endif

  // Null `StringPieceImpl` constructor.
  constexpr StringPieceImpl() noexcept: ptr_(nullptr), len_(0) {}

  // Implicit constructor of a `StringPieceImpl' from NUL-terminated `str'.
  constexpr StringPieceImpl(const_pointer s) // NOLINT
      : ptr_(s), len_(s ? traits_type::length(s) : 0) {}

  // Implicit constructor of a `StringPieceImpl' from a `const char*' and `n'.
  constexpr StringPieceImpl(const Char* s, size_t n)
      : ptr_(s), len_(n) {}

  // Implicit constructors of a `StringPieceImpl' from `std::basic_string'.
  template<typename Alloc>
  StringPieceImpl(const std::basic_string<Char, traits_type, Alloc>& s) // NOLINT
      noexcept : StringPieceImpl(s.data(), s.size()) {}

  // constexpr StringPieceImpl(const StringPieceImpl&) noexcept = default;
  // StringPieceImpl& operator=(const StringPieceImpl&) noexcept = default;

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
  // No bounds checking is performed, the behavior is undefined if
  // pos >= size().
  constexpr const_reference operator[](size_type pos) const {
    return OAK_ASSERT(pos < size()), ptr_[pos];
  }

  // Resturn a const reference to the character at specified location pos.
  // Bounds checking is performed, exception of type `std::out_of_range'
  // will be thrown on invalid access.
  constexpr const_reference at(size_type pos) const {
    return OAK_ASSERT(pos < size()), ptr_[pos];
  }

  // Return reference to the first character in the view. the behavior is
  // undefined if empty() == true.
  constexpr const_reference front() const {
    return OAK_ASSERT(!empty()), ptr_[0];
  }

  // Return reference to the last character in the view. the behavior is
  // undefined if empty() == true.
  constexpr const_reference back() const {
    return OAK_ASSERT(!empty()), ptr_[size() - 1];
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
    OAK_ASSERT(n <= len_);
    ptr_ += n;
    len_ -= n;
  }

  // Moves the end of the view back by n characters. the behavior is
  // undefined if n > size().
  void remove_suffix(size_type n) {
    OAK_ASSERT(n <= len_);
    len_ -= n;
  }

  // Exchanges the view with that of other.
  void swap(StringPieceImpl& other) noexcept {
    auto tmp = *this;
    *this = other;
    other = tmp;
  }

  // Operations

  // Copies the substring[pos, pos + rcount) to the character array pointed
  // to by dest, where rcount is the samller of count and size() - pos.
  size_type copy(Char* buf, size_type n, size_type pos = 0) const {
    if (OAK_UNLIKELY(pos > len_))
      throw std::out_of_range("polly::string_view::copy");

    n = std::min(len_ - pos, n);
    if (n > 0)
      traits_type::copy(buf, ptr_ + pos, n);
    return n;
  }

  // Returns a view of the substring[pos, pos + rcount), where rcount is the
  // smaller of count and size() - pos.
  constexpr StringPieceImpl substr(size_type pos = 0,
                                   size_type n = npos) const {
    return StringPieceImpl(ptr_ + pos, n < len_ - pos ? n : len_ - pos);
  }

  // Compares tow character sequences, negative value if this view is less
  // than the other character sequence, zero if the both character sequences
  // are equal, positive value if this view is greater than the other
  // character sequence.
  constexpr int compare(StringPieceImpl other) const noexcept {
    return CompareHelper(len_, other.len_,
        std::min(len_, other.len_) == 0 ? 0
          : traits_type::compare(ptr_, other.ptr_, std::min(len_, other.len_)));
  }

  constexpr int compare(
      size_type pos1, size_type count1, StringPieceImpl other) const {
    return substr(pos1, count1).compare(other);
  }

  constexpr int compare(
      size_type pos1, size_type count1,
      StringPieceImpl other, size_type pos2, size_type count2) const {
    return substr(pos1, count1).compare(other.substr(pos2, count2));
  }

  constexpr int compare(const Char* s) const {
    return compare(StringPieceImpl(s));
  }

  constexpr int compare(
      size_type pos1, size_type count1, const Char* s) const {
    return substr(pos1, count1).compare(StringPieceImpl(s));
  }

  constexpr int compare(size_type pos1, size_type count1,
      const Char* s, size_type pos2, size_type count2) const {
    return substr(pos1, count1)
        .compare(StringPieceImpl(s).substr(pos2, count2));
  }

  // Checks if the string view begins with the given prefix. true if
  // string view begin with the provided prefix, false otherwise.
  constexpr bool starts_with(StringPieceImpl other) const noexcept {
    return size() >= other.size() && !compare(0, other.size(), other);
  }

  constexpr bool starts_with(Char c) const noexcept {
    return !empty() && traits_type::eq(front(), c);
  }

  constexpr bool starts_with(const Char* s) const noexcept {
    return starts_with(StringPieceImpl(s));
  }

  constexpr bool starts_with(const Char* s, size_type n) const noexcept {
    return starts_with(StringPieceImpl(s, n));
  }

  // Checks if the string view ends with the given suffix. true if
  // string view ends with the provided prefix, false otherwise.
  constexpr bool ends_with(StringPieceImpl other) const noexcept {
    return size() >= other.size() &&
        !compare(size() - other.size(), npos, other);
  }

  constexpr bool ends_with(Char c) const noexcept {
    return !empty() && traits_type::eq(back(), c);
  }

  constexpr bool ends_with(const Char* s) const noexcept {
    return ends_with(StringPieceImpl(s));
  }

  constexpr bool ends_with(const Char* s, size_type n) const noexcept {
    return ends_with(StringPieceImpl(s, n));
  }

  // StringPieceImpl::find

  // Find the first substring equal to the given character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the first character of the found substring, or `npos'
  // if no such substring is found.
  size_type find(StringPieceImpl v, size_type pos = 0) const noexcept;
  size_type find(Char c, size_type pos = 0) const noexcept;
  size_type find(const Char* s, size_type pos, size_type count) const {
    return find(StringPieceImpl(s, count), pos);
  }
  size_type find(const Char* s, size_type pos = 0) const {
    return find(StringPieceImpl(s), pos);
  }

  // Find the last substring equal to the given character sequence.
  // @v      view to search for
  // @pos    position at which to start the search
  // @count  length substring to search for
  // @s      pointer to a character string to search for
  // @ch     character to searchc for
  // @return Position of the first character of the found substring, or `npos'
  // if no such substring is found.
  size_type rfind(StringPieceImpl v, size_type pos = npos) const noexcept;
  size_type rfind(Char c, size_type pos = npos) const noexcept;
  size_type rfind(const Char* s, size_type pos, size_type count) const {
    return rfind(StringPieceImpl(s, count), pos);
  }
  size_type rfind(const Char* s, size_type pos = npos) const {
    return rfind(StringPieceImpl(s), pos);
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
  size_type find_first_of(StringPieceImpl s, size_type pos = 0) const noexcept;

  size_type find_first_of(Char c, size_type pos = 0) const noexcept {
    return find(c, pos);
  }

  size_type find_first_of(const Char* s, size_type pos, size_type count) const {
    return find_first_of(string_view(s, count), pos);
  }

  size_type find_first_of(const Char* s, size_type pos = 0) const {
    return find_first_of(StringPieceImpl(s), pos);
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
  size_type find_last_of(StringPieceImpl s,
                         size_type pos = npos) const noexcept;

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
  size_type find_first_not_of(StringPieceImpl s,
                              size_type pos = 0) const noexcept;

  size_type find_first_not_of(Char c, size_type pos = 0) const noexcept;

  size_type find_first_not_of(const Char* s, size_type pos,
                              size_type count) const {
    return find_first_not_of(StringPieceImpl(s, count), pos);
  }

  size_type find_first_not_of(const Char* s, size_type pos = 0) const {
    return find_first_not_of(StringPieceImpl(s), pos);
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
  size_type find_last_not_of(StringPieceImpl s,
      size_type pos = npos) const noexcept;

  size_type find_last_not_of(Char c, size_type pos = npos) const noexcept;

  size_type find_last_not_of(const Char* s, size_type pos,
      size_type count) const {
    return find_last_not_of(string_view(s, count), pos);
  }

  size_type find_last_not_of(const Char* s, size_type pos = npos) const {
    return find_last_not_of(StringPieceImpl(s), pos);
  }

  // Explicit converts to std::basic_string.
  template <typename Alloc>
  explicit operator std::basic_string<Char, traits_type, Alloc>() const {
    if (!data()) return {};
    return std::basic_string<Char, traits_type, Alloc>(data(), size());
  }

 private:
#if defined(__cpp_inline_variables)
  static inline constexpr size_type kMaxSize =
      std::numeric_limits<difference_type>::max() / sizeof(Char);
#else
  static constexpr size_type kMaxSize =
      std::numeric_limits<difference_type>::max() / sizeof(Char);
#endif

  static constexpr int CompareHelper(size_type a, size_type b, int r) {
    return r == 0 ? static_cast<int>(a > b) - static_cast<int>(a < b)
        : (r < 0 ? -1 : 1);
  }

  const_pointer ptr_;
  size_type len_;
};

#if !defined(__cpp_inline_variables)
template<typename Char, typename Traits>
 constexpr typename StringPieceImpl<Char, Traits>::size_type
    StringPieceImpl<Char, Traits>::npos;

template<typename Char, typename Traits>
 constexpr typename StringPieceImpl<Char, Traits>::size_type
    StringPieceImpl<Char, Traits>::kMaxSize;
#endif

namespace stringpiece_internal {

template<typename Char, typename Traits>
const char* FindHelper(const Char* haystack, size_t haylen,
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
}  // namespace stringpiece_internal

// StringPieceImpl::find
template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find(
    StringPieceImpl v, size_type pos) const noexcept {
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
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find(Char c, size_type pos) const noexcept {
  size_type ret = npos;
  if (pos < len_) {
    const Char* p = traits_type::find(ptr_ + pos, len_ - pos, c);
    if (p)
      ret = p - ptr_;
  }
  return ret;
}

// StringPieceImpl::rfind
template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::rfind(
    StringPieceImpl v, size_type pos) const noexcept {
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
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::rfind(Char c, size_type pos) const noexcept {
  if (len_ > 0) {
    for (pos = std::min(len_ - 1, pos) + 1; pos > 0; --pos) {
      if (traits_type::eq(ptr_[pos - 1], c))
        return pos - 1;
    }
  }
  return npos;
}

// StringPieceImpl::find_first_of
template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find_first_of(
    StringPieceImpl v, size_type pos) const noexcept {
  if (v.len_ > 0) {
    for (; pos < len_; ++pos) {
      if (traits_type::find(v.ptr_, v.len_, ptr_[pos]))
        return pos;
    }
  }
  return npos;
}

// StringPieceImpl::find_last_of
template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find_last_of(
    StringPieceImpl v, size_type pos) const noexcept {
  if (len_ > 0 && v.len_ > 0) {
    for (pos = std::min(pos, len_ - 1) + 1; pos > 0; --pos) {
      if (traits_type::find(v.ptr_, v.len_, ptr_[pos - 1]))
        return pos - 1;
    }
  }
  return npos;
}

// StringPieceImpl::find_first_not_of
template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find_first_not_of(
    StringPieceImpl v, size_type pos) const noexcept {
  for (; pos < len_; ++pos) {
    if (!traits_type::find(v.ptr_, v.len_, ptr_[pos]))
      return pos;
  }
  return npos;
}

template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find_first_not_of(
    Char c, size_type pos) const noexcept {
  for (; pos < len_; ++pos) {
    if (!traits_type::eq(c, ptr_[pos]))
      return pos;
  }
  return npos;
}

// StringPieceImpl::find_last_not_of
template<typename Char, typename Traits>
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find_last_not_of(
    StringPieceImpl v, size_type pos) const noexcept {
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
typename StringPieceImpl<Char, Traits>::size_type
StringPieceImpl<Char, Traits>::find_last_not_of(
    Char c, size_type pos) const noexcept {
  if (len_ > 0) {
    for (pos = std::min(pos, len_ - 1) + 1; pos > 0; --pos) {
      if (!traits_type::eq(c, ptr_[pos - 1]))
        return pos - 1;
    }
  }
  return npos;
}

constexpr bool operator==(StringPiece lhs, StringPiece rhs) noexcept {
  return lhs.compare(rhs) == 0;
}

constexpr bool operator!=(StringPiece lhs, StringPiece rhs) noexcept {
  return !(lhs == rhs);
}

constexpr bool operator<(StringPiece lhs, StringPiece rhs) noexcept {
  return lhs.compare(rhs) < 0;
}

constexpr bool operator>(StringPiece lhs, StringPiece rhs) noexcept {
  return rhs < lhs;
}

constexpr bool operator<=(StringPiece lhs, StringPiece rhs) noexcept {
  return !(rhs < lhs);
}

constexpr bool operator>=(StringPiece lhs, StringPiece rhs) noexcept {
  return !(lhs < rhs);
}

namespace stringpiece_internal {
template<typename Char, typename Traits>
void WritePadding(std::basic_ostream<Char, Traits>& o, size_t pad) { // NOLINT
  char fill_buf[32];
  memset(fill_buf, o.fill(), 32);
  while (pad) {
    size_t n = std::min(pad, sizeof(fill_buf));
    o.write(fill_buf, n);
    pad -= n;
  }
}
}  // namespace stringpiece_internal

// IO Insertion Operator
template<typename Char, typename Traits>
std::basic_ostream<Char, Traits>& operator<<(
    std::basic_ostream<Char, Traits>& o, StringPieceImpl<Char, Traits> v) {
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
      stringpiece_internal::WritePadding(o, lpad);
    o.write(v.data(), v.size());
    if (rpad)
      stringpiece_internal::WritePadding(o, rpad);
    o.width(0);
  }
  return o;
}

namespace stringpiece_internal {
template<typename Char, typename Traits>
struct hash_base {
  size_t operator()(StringPieceImpl<Char, Traits> v) {
    // TODO(yuyue): This way is not portable.
    return v.size() == 0 ? 0 : std::_Hash_impl::hash(v.data(), v.size());
  }
};
}  // namespace stringpiece_internal
}  // namespace oak

namespace std {
template<typename Char, typename Traits>
struct hash<oak::StringPieceImpl<Char, Traits>>
    : public oak::stringpiece_internal::hash_base<Char, Traits> {};
}  // namespace std

#endif  // OAK_COMMON_STRINGPIECE_H_
