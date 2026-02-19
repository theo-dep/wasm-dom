#pragma once

#include <algorithm>
#include <ranges>
#include <string>

namespace wasmdom::internals
{
    inline std::string lower(std::string str)
    {
        static const auto tolower{
            [](unsigned char c) {
                return static_cast<char>(std::tolower(c));
            }
        };
        std::ranges::copy(std::views::transform(str, tolower), str.begin());
        return str;
    }

    inline std::string upper(std::string str)
    {
        static const auto toupper{
            [](unsigned char c) {
                return static_cast<char>(std::toupper(c));
            }
        };
        std::ranges::copy(std::views::transform(str, toupper), str.begin());
        return str;
    }
}
