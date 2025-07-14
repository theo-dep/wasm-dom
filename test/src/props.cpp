#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("props", "[props]")
{
    setupDom();

    SECTION("should create element with prop")
    {
        ScopedVNode vnode{
            h("div",
              { { "src", emscripten::val("http://localhost/") } })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://localhost/")));
    }

    SECTION("changes an elements props")
    {
        VNode* vnode = h("a",
                         { { "src", emscripten::val("http://other/") } });
        ScopedVNode vnode2{
            h("a",
              { { "src", emscripten::val("http://localhost/") } })
        };

        patch(getRoot(), vnode);
        patch(vnode, vnode2.get());

        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://localhost/")));
    }

    SECTION("preserves memoized props")
    {
        Attributes attrs = {
            { "src", emscripten::val("http://other/") }
        };
        ScopedVNode vnode{ h("a", attrs) };
        ScopedVNode vnode2{ h("a", attrs) };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://other/")));

        patch(vnode.release(), vnode2.get());

        elm = getBodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://other/")));
    }

    SECTION("removes an elements props")
    {
        VNode* vnode = h("a",
                         { { "src", emscripten::val("http://other/") } });
        ScopedVNode vnode2{ h("a") };

        patch(getRoot(), vnode);
        patch(vnode, vnode2.get());

        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val::undefined()));
    }

    SECTION("should update value prop if user interacted with the element")
    {
        ScopedVNode vnode{
            h("input",
              { { "value", emscripten::val("foo") } })
        };
        ScopedVNode vnode2{
            h("input",
              { { "value", emscripten::val("foo") } })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["value"].strictlyEquals(emscripten::val("foo")));

        elm.set("value", emscripten::val("bar"));
        REQUIRE(elm["value"].strictlyEquals(emscripten::val("bar")));

        patch(vnode.release(), vnode2.get());

        REQUIRE(elm["value"].strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should update checked prop if user interacted with the element")
    {
        ScopedVNode vnode{
            h("input",
              { { "type", "checkbox" },
                { "checked", emscripten::val(true) } })
        };
        ScopedVNode vnode2{
            h("input",
              { { "type", "checkbox" },
                { "checked", emscripten::val(true) } })
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["checked"].strictlyEquals(emscripten::val(true)));

        elm.set("checked", emscripten::val(false));
        REQUIRE(elm["checked"].strictlyEquals(emscripten::val(false)));

        patch(vnode.release(), vnode2.get());

        REQUIRE(elm["checked"].strictlyEquals(emscripten::val(true)));
    }
}
