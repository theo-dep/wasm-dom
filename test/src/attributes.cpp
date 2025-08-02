#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("attributes", "[attributes]")
{
    setupDom();

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

    SECTION("should have their provided values")
    {
        VNode vnode =
            div(("href", "/foo"s),
                ("minlength", "1"s),
                ("value", "foo"s));

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
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

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));

        VNode vnode2 = div(data);

        vdom.patch(vnode2);

        elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should be omitted when falsy values are provided")
    {
        VNode vnode =
            div(("href", "null"s),
                ("minlength", "0"s),
                ("value", "false"s));

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val::null()));
    }

    SECTION("should set truthy values to empty string")
    {
        VNode vnode =
            input(
                ("href", "null"s),
                ("minlength", "0"s),
                ("readonly", "true"s)
            );

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("readonly")).strictlyEquals(emscripten::val("")));
    }

    SECTION("should be set correctly when namespaced")
    {
        VNode vnode = div(("xlink:href", "#foo"s));

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(
            elm.call<emscripten::val>(
                "getAttributeNS",
                emscripten::val("http://www.w3.org/1999/xlink"),
                emscripten::val("href")
            ) == emscripten::val("#foo")
        );
    }
}
