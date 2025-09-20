#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("First use of single header", "[single-header][1]")
{
    const JSDom jsDom;

    SECTION("should patch a node")
    {
        VNode vnode = span();
        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(jsDom.document()["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["nodeName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["className"].strictlyEquals(emscripten::val("")));
    }
}
