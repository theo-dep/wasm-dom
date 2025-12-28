#pragma once

#include <catch2/matchers/catch_matchers_templated.hpp>

#include <emscripten/val.h>

struct StrictlyEqualsMatcher : Catch::Matchers::MatcherGenericBase
{
    StrictlyEqualsMatcher(const emscripten::val& val);

    bool match(const emscripten::val& other) const;

    std::string describe() const override;

private:
    const emscripten::val& _val;
};

StrictlyEqualsMatcher StrictlyEquals(const emscripten::val& val);

namespace Catch
{
    template <>
    struct StringMaker<emscripten::val>
    {
        static std::string convert(const emscripten::val& val);
    };
}
