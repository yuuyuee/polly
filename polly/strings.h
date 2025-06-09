// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#ifndef POLLY_COMMON_STRINGS_H_
#define POLLY_COMMON_STRINGS_H_

#include <cctype>
#include <string>
#include <vector>
#include <map>
#include <string_view>
#include <algorithm>
#include <iterator>
#include <type_traits>

namespace polly {
std::map<std::string, std::string> ParseKeyValue(const std::string& str);

inline std::string_view TrimLeft(std::string_view str) {
    while (!str.empty() && isspace(str.front()))
        str.remove_prefix(1);
    return str;
}

inline std::string_view TrimRight(std::string_view str) {
    while (!str.empty() && isspace(str.back()))
        str.remove_suffix(1);
    return str;
}

inline std::string_view Trim(std::string_view str) {
    return TrimLeft(TrimRight(str));
}

inline std::string ToLower(const std::string_view& str) {
    std::string res;
    std::transform(std::cbegin(str), std::cend(str),
            std::back_inserter(res), tolower);
    return res;
}

inline std::string ToUpper(const std::string_view& str) {
    std::string res;
    std::transform(std::cbegin(str), std::cend(str),
            std::back_inserter(res), toupper);
    return res;
}

inline const char* SafeCstr(const std::string& str) {
    return str.empty() ? nullptr : str.c_str();
}

template <typename Iter>
inline std::string Join(Iter first, Iter last,
                        const std::string_view &delimit) {
    static_assert(
        std::is_convertible_v<
            std::string,
            typename std::iterator_traits<Iter>::value_type
        >,
        "Join");
    std::string res;
    for (; first != last; ++first) {
        if (!res.empty())
            res.append(delimit.data(), delimit.size());
        res.append(*first);
    }
    return res;
}

template <typename Container>
inline std::string Join(const Container& container,
                        const std::string_view& delimit) {
    return Join(container.begin(), container.end(), delimit);
}


}  // namespace polly

#endif  // POLLY_COMMON_STRINGS_H_
