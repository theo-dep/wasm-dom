#include <catch2/catch_test_macros.hpp>

#include "internals/domapi.hpp"

#include "jsdom.hpp"

using namespace wasmdom::internals;

TEST_CASE("domApi", "[domApi]")
{
    const JSDom jsDom;

    SECTION("Should remove an unknown node")
    {
        domapi::removeNode(emscripten::val::null());
        domapi::removeNode(emscripten::val::undefined());
    }
}
