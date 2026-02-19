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
        const emscripten::val parent = jsDom.body();

        domapi::removeNode(parent, emscripten::val::null());
        domapi::removeNode(parent, emscripten::val::undefined());
    }
}
