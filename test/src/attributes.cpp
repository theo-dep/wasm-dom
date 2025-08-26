#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("attributes", "[attributes]")
{
    const JSDom jsDom;

    SECTION("can be copied")
    {
        Attributes data;
        data.attrs = {
            { "href", "/foo" },
            { "minlength", "1" },
            { "value", "foo" }
        };
        Attributes other(data);
        REQUIRE(data.attrs == other.attrs);
    }

    SECTION("can be moved")
    {
        Attributes data;
        data.attrs = {
            { "href", "/foo" },
            { "minlength", "1" },
            { "value", "foo" }
        };
        Attributes moved(data);
        Attributes other(std::move(moved));
        REQUIRE(data.attrs == other.attrs);
    }

    SECTION("should have their provided values")
    {
        VNode vnode =
            div(("href", "/foo"s),
                ("minlength", "1"s),
                ("value", "foo"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("can be memoized")
    {
        Attributes data;
        data.attrs = {
            { "href", "/foo" },
            { "minlength", "1" },
            { "value", "foo" }
        };
        VNode vnode = div(data);

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));

        VNode vnode2 = div(data);

        vdom.patch(vnode2);

        node = jsDom.bodyFirstChild();

        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should be omitted when falsy values are provided")
    {
        VNode vnode =
            div(("href", "null"s),
                ("minlength", "0"s),
                ("value", "false"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("value")).isNull());
    }

    SECTION("should set truthy values to empty string")
    {
        VNode vnode =
            input(
                ("href", "null"s),
                ("minlength", "0"s),
                ("readonly", "true"s)
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("readonly")).strictlyEquals(emscripten::val("")));
    }

    SECTION("should be set correctly when xlink namespaced")
    {
        VNode vnode = div(("xlink:href", "#foo"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE(
            node.call<emscripten::val>(
                "getAttributeNS",
                emscripten::val("http://www.w3.org/1999/xlink"),
                emscripten::val("href")
            ) == emscripten::val("#foo")
        );
    }

    SECTION("should be set correctly when xml namespaced")
    {
        VNode vnode = div(("xml:base", "#foo"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE(
            node.call<emscripten::val>(
                "getAttributeNS",
                emscripten::val("http://www.w3.org/XML/1998/namespace"),
                emscripten::val("base")
            ) == emscripten::val("#foo")
        );
    }
}
