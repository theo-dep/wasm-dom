#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "catch_emscripten.hpp"
#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("dataset", "[dataset]")
{
    const JSDom jsDom;

    SECTION("should set on initial element creation")
    {
        VNode vnode = div(("data-foo", "foo"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")), StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should update dataset")
    {
        VNodeAttributes data;
        data.attrs = { { "data-foo", "foo" },
                       { "data-bar", "bar" } };
        VNode vnode = i(data);

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")), StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-bar")), StrictlyEquals(emscripten::val("bar")));

        VNode vnode2 = i(data);

        vdom.patch(vnode2);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")), StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-bar")), StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("can be memorized")
    {
        VNode vnode =
            div(("data-foo", "foo"s),
                ("data-bar", "bar"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")), StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-bar")), StrictlyEquals(emscripten::val("bar")));

        VNode vnode2 = div(("data-baz", "baz"s));

        vdom.patch(vnode2);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-baz")), StrictlyEquals(emscripten::val("baz")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).isNull());
    }
}
