#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("dataset", "[dataset]")
{
    setupDom();

    SECTION("should set on initial element creation")
    {
        ScopedVNode vnode{
            h("div", { { "data-foo", "foo" } })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should update dataset")
    {
        Attributes attrs = {
            { "data-foo", "foo" },
            { "data-bar", "bar" }
        };
        ScopedVNode vnode{ h("i", attrs) };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("bar")));

        ScopedVNode vnode2{ h("i", attrs) };

        patch(vnode.release(), vnode2.get());

        elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("bar")));
    }

    SECTION("can be memorized")
    {
        ScopedVNode vnode{
            h("div", { { "data-foo", "foo" },
                       { "data-bar", "bar" } })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("bar")));

        ScopedVNode vnode2{
            h("div", { { "data-baz", "baz" } })
        };

        patch(vnode.release(), vnode2.get());

        elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-baz")).strictlyEquals(emscripten::val("baz")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val::null()));
    }
}
