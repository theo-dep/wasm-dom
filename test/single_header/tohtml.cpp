#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("toHTML", "[single-header][toHTML]")
{
    SECTION("should parse elements")
    {
        VNode vnode = div();
        REQUIRE(vnode.toHTML() == "<div></div>");
    }
}
