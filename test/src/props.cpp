#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("props", "[props]")
{
    const JSDom jsDom;

    SECTION("should create element with prop")
    {
        VNode vnode = div(("src", emscripten::val("http://localhost/")));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val elm = jsDom.bodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://localhost/")));
    }

    SECTION("changes an elements props")
    {
        VNode vnode = a(("src", emscripten::val("http://other/")));
        VNode vnode2 = a(("src", emscripten::val("http://localhost/")));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        vdom.patch(vnode2);

        emscripten::val elm = jsDom.bodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://localhost/")));
    }

    SECTION("preserves memorized props")
    {
        VNodeAttributes data;
        data.props = { { "src", emscripten::val("http://other/") } };
        VNode vnode = a(data);
        VNode vnode2 = a(data);

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val elm = jsDom.bodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://other/")));

        vdom.patch(vnode2);

        elm = jsDom.bodyFirstChild();
        REQUIRE(elm["src"].strictlyEquals(emscripten::val("http://other/")));
    }

    SECTION("removes an elements props")
    {
        VNode vnode = a(("src", emscripten::val("http://other/")));
        VNode vnode2 = a();

        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        vdom.patch(vnode2);

        emscripten::val elm = jsDom.bodyFirstChild();
        REQUIRE(elm["src"].isUndefined());
    }

    SECTION("should update value prop if user interacted with the element")
    {
        VNode vnode = input(("value", emscripten::val("foo")));
        VNode vnode2 = input(("value", emscripten::val("foo")));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val elm = jsDom.bodyFirstChild();
        REQUIRE(elm["value"].strictlyEquals(emscripten::val("foo")));

        elm.set("value", emscripten::val("bar"));
        REQUIRE(elm["value"].strictlyEquals(emscripten::val("bar")));

        vdom.patch(vnode2);

        REQUIRE(elm["value"].strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should update checked prop if user interacted with the element")
    {
        VNode vnode =
            input(
                ("type", "checkbox"s),
                ("checked", emscripten::val(true))
            );
        VNode vnode2 =
            input(
                ("type", "checkbox"s),
                ("checked", emscripten::val(true))
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val elm = jsDom.bodyFirstChild();
        REQUIRE(elm["checked"].strictlyEquals(emscripten::val(true)));

        elm.set("checked", emscripten::val(false));
        REQUIRE(elm["checked"].strictlyEquals(emscripten::val(false)));

        vdom.patch(vnode2);

        REQUIRE(elm["checked"].strictlyEquals(emscripten::val(true)));
    }
}
