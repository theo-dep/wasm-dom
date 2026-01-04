#include <catch2/catch_test_macros.hpp>

#include "internals/domapi.hpp"

#include "catch_emscripten.hpp"
#include "jsdom.hpp"

using namespace wasmdom::internals;

TEST_CASE("domApi", "[domApi]")
{
    const JSDom jsDom;

    SECTION("should remove an unknown node")
    {
        domapi::removeNode(emscripten::val::null());
        domapi::removeNode(emscripten::val::undefined());
    }

    SECTION("should get from null parent node")
    {
        REQUIRE_THAT(domapi::parentNode(emscripten::val::null()), StrictlyEquals(emscripten::val::null()));
        REQUIRE_THAT(domapi::parentNode(emscripten::val::undefined()), StrictlyEquals(emscripten::val::null()));
    }

    SECTION("should get from null nextSibling node")
    {
        REQUIRE_THAT(domapi::nextSibling(emscripten::val::null()), StrictlyEquals(emscripten::val::null()));
        REQUIRE_THAT(domapi::nextSibling(emscripten::val::undefined()), StrictlyEquals(emscripten::val::null()));
    }
}
