#include "catch_emscripten.hpp"

#include <emscripten/em_js.h>

EM_JS_DEPS(deps, "$stringToNewUTF8");
EM_JS(char*, asString, (emscripten::EM_VAL handle), { return stringToNewUTF8(String(Emval.toValue(handle))); });

StrictlyEqualsMatcher::StrictlyEqualsMatcher(const emscripten::val& val)
    : _val{ val }
{
}

bool StrictlyEqualsMatcher::match(const emscripten::val& other) const
{
    return _val.strictlyEquals(other);
}

std::string StrictlyEqualsMatcher::describe() const
{
    return "Equals: " + std::string(asString(_val.as_handle()));
}

StrictlyEqualsMatcher StrictlyEquals(const emscripten::val& val)
{
    return StrictlyEqualsMatcher{ val };
}

std::string Catch::StringMaker<emscripten::val>::convert(const emscripten::val& val)
{
    return asString(val.as_handle());
}
