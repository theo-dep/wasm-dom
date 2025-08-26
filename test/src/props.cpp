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

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["src"].strictlyEquals(emscripten::val("http://localhost/")));
    }

    SECTION("changes an elements props")
    {
        VNode vnode = a(("src", emscripten::val("http://other/")));
        VNode vnode2 = a(("src", emscripten::val("http://localhost/")));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        vdom.patch(vnode2);

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["src"].strictlyEquals(emscripten::val("http://localhost/")));
    }

    SECTION("preserves memorized props")
    {
        Attributes data;
        data.props = { { "src", emscripten::val("http://other/") } };
        VNode vnode = a(data);
        VNode vnode2 = a(data);

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["src"].strictlyEquals(emscripten::val("http://other/")));

        vdom.patch(vnode2);

        node = jsDom.bodyFirstChild();
        REQUIRE(node["src"].strictlyEquals(emscripten::val("http://other/")));
    }

    SECTION("removes an elements props")
    {
        VNode vnode = a(("src", emscripten::val("http://other/")));
        VNode vnode2 = a();

        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        vdom.patch(vnode2);

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["src"].isUndefined());
    }

    SECTION("should update value prop if user interacted with the element")
    {
        VNode vnode = input(("value", emscripten::val("foo")));
        VNode vnode2 = input(("value", emscripten::val("foo")));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["value"].strictlyEquals(emscripten::val("foo")));

        node.set("value", emscripten::val("bar"));
        REQUIRE(node["value"].strictlyEquals(emscripten::val("bar")));

        vdom.patch(vnode2);

        REQUIRE(node["value"].strictlyEquals(emscripten::val("foo")));
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

        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["checked"].strictlyEquals(emscripten::val(true)));

        node.set("checked", emscripten::val(false));
        REQUIRE(node["checked"].strictlyEquals(emscripten::val(false)));

        vdom.patch(vnode2);

        REQUIRE(node["checked"].strictlyEquals(emscripten::val(true)));
    }
}
