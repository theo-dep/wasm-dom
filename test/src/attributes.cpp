#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#ifdef __EMSCRIPTEN__
#include "catch_emscripten.hpp"
#include "jsdom.hpp"
#endif

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("attributes", "[attributes]")
{
    SECTION("can be copied")
    {
        VNodeAttributes data;
        data.attrs = {
            { "href", "/foo" },
            { "minlength", "1" },
            { "value", "foo" }
        };
        VNodeAttributes other(data);
        REQUIRE(data.attrs == other.attrs);
    }

    SECTION("can be moved")
    {
        VNodeAttributes data;
        data.attrs = {
            { "href", "/foo" },
            { "minlength", "1" },
            { "value", "foo" }
        };
        VNodeAttributes moved(data);
        VNodeAttributes other(std::move(moved));
        REQUIRE(data.attrs == other.attrs);
    }

#ifdef __EMSCRIPTEN__

    const JSDom jsDom;

    SECTION("should have their provided values")
    {
        VNode vnode =
            div(("href", "/foo"s),
                ("minlength", "1"s),
                ("value", "foo"s));

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("href")), StrictlyEquals(emscripten::val("/foo")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")), StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("value")), StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("can be memoized")
    {
        VNodeAttributes data;
        data.attrs = {
            { "href", "/foo" },
            { "minlength", "1" },
            { "value", "foo" }
        };
        VNode vnode = div(data);

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("href")), StrictlyEquals(emscripten::val("/foo")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")), StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("value")), StrictlyEquals(emscripten::val("foo")));

        VNode vnode2 = div(data);

        vdom.patch(vnode2);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("href")), StrictlyEquals(emscripten::val("/foo")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")), StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("value")), StrictlyEquals(emscripten::val("foo")));
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

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("href")), StrictlyEquals(emscripten::val("null")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")), StrictlyEquals(emscripten::val("0")));
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

        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("href")), StrictlyEquals(emscripten::val("null")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("minlength")), StrictlyEquals(emscripten::val("0")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("readonly")), StrictlyEquals(emscripten::val("")));
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

#endif
}
