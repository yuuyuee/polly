// Copyright RCT Power 2025
// Author: ivan.yu (ivan.yu@rct-power.com.cn)

#include "rctems/common/strings.h"

#include <string>
#include <regex>  // NOLINT
#include <map>

namespace rctems {

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

}  // namespace rctems
