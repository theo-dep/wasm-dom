#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

using namespace wasmdom;

TEST_CASE("domApi", "[domApi]")
{
    const JSDom jsDom;

    SECTION("should access an unknown node")
    {
        REQUIRE(domapi::node(-1).isNull());
    }

    SECTION("Should remove an unknown node")
    {
        domapi::removeChild(-1);
    }
}
