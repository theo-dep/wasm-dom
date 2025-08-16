#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

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

        emscripten::val elm = jsDom.bodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should update dataset")
    {
        VNodeAttributes data;
        data.attrs = { { "data-foo", "foo" },
                       { "data-bar", "bar" } };
        VNode vnode = i(data);

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val elm = jsDom.bodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("bar")));

        VNode vnode2 = i(data);

        vdom.patch(vnode2);

        elm = jsDom.bodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("bar")));
    }

    SECTION("can be memorized")
    {
        VNode vnode =
            div(("data-foo", "foo"s),
                ("data-bar", "bar"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val elm = jsDom.bodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("bar")));

        VNode vnode2 = div(("data-baz", "baz"s));

        vdom.patch(vnode2);

        elm = jsDom.bodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-baz")).strictlyEquals(emscripten::val("baz")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).isNull());
    }
}
