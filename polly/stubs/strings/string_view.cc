#if !defined(POLLY_HAVE_STD_STRING_VIEW)
#include "stubs/strings/string_view.h"

namespace polly {
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
    return pos < len_ ? pos : npos;

  if (v.len_ <= len_) {
    for (; pos <= len_ - v.len_; ++pos) {
      if (traits_type::eq(ptr_[pos], v.ptr[0]) &&
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
        return pos;
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
      if (traits_type::find(v.ptr_, v.len_, ptr_[pos]))
        return pos;
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
  return npos;
}

// basic_string_view::find_last_not_of
template<typename Char, typename Traits>
typename basic_string_view<Char, Traits>::size_type
basic_string_view<Char, Traits>::find_last_not_of(
    basic_string_view v, size_type pos) const noexcept {
  if (len_ > 0 && v.len_ > 0) {
    for (pos = std::min(pos, len_ - 1) + 1; pos > 0; --pos) {
      if (!traits_type::find(v.ptr_, v.len_, ptr_[pos]))
        return pos;
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
      if (!traits_type::eq(c, ptr_[pos]))
        return pos;
    }
  }
  return npos;
}



} // namespace polly
#endif // POLLY_HAVE_STD_STRING_VIEW