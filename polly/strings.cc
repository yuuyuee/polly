// Copyright (C) 2025 Ivan Yu (yuyue2200@hotmail.com)
//
// This file is placed under the LGPL.  Please see the file
// COPYING for more details.
//
// SPDX-License-Identifier: LGPL-2.1

#include "polly/strings.h"

#include <string>
#include <regex>  // NOLINT
#include <map>

namespace polly {

std::map<std::string, std::string> ParseKeyValue(const std::string& str) {
    static const std::regex pattern(R"(([^=;]+)=([^;]+))");

    std::map<std::string, std::string> res;
    std::sregex_iterator it(str.begin(), str.end(), pattern);
    std::sregex_iterator end;

    for (; it != end; ++it) {
        std::smatch match = *it;
        std::string key = match[1].str();
        std::string value = match[2].str();
        res[key] = value;
    }
    return res;
}

}  // namespace polly
