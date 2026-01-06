#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "catch_emscripten.hpp"
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
        REQUIRE_THAT(jsDom.document()["body"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["className"], StrictlyEquals(emscripten::val("")));
    }
}
