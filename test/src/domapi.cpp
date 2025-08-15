#include <catch2/catch_test_macros.hpp>

#include "wasm-dom/domapi.hpp"

#include "jsdom.hpp"

using namespace wasmdom;

TEST_CASE("domApi", "[domApi]")
{
    const JSDom jsDom;

    SECTION("Should remove an unknown node")
    {
        domapi::removeChild(emscripten::val::null());
        domapi::removeChild(emscripten::val::undefined());
    }
}
