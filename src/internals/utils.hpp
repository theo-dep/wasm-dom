#pragma once

#include <algorithm>
#include <ranges>
#include <string>

namespace wasmdom::internals
{
    inline void lower(std::string& str)
    {
        static const auto tolower{
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        };
        std::ranges::copy(std::views::transform(str, tolower), str.begin());
    }

    inline std::string upper(const std::string& str)
    {
        static const auto toupper{
            [](unsigned char c) {
                return static_cast<char>(std::toupper(c));
            }
        };
        std::string upperStr = str;
        std::ranges::copy(std::views::transform(str, toupper), upperStr.begin());
        return upperStr;
    }
}
