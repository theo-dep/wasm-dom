#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("domApi", "[domApi]")
{
    setupDom();

    SECTION("should access an unknown node")
    {
        REQUIRE(domapi::node(-1).isNull());
    }

    SECTION("Should remove an unknown node")
    {
        domapi::removeChild(-1);
    }
}
