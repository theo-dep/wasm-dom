#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("attributes", "[attributes]")
{
    setupDom();

    SECTION("should have their provided values")
    {
        VNode* vnode{
            h("div", Data(
                         Attrs{
                             { "href", "/foo" },
                             { "minlength", "1" },
                             { "value", "foo" } }))
        };

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("can be memoized")
    {
        Data data = Data(
            Attrs{
                { "href", "/foo" },
                { "minlength", "1" },
                { "value", "foo" } });
        VNode* vnode{ h("div", data) };

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));

        VNode* vnode2{ h("div", data) };

        vdom.patch(vnode2);

        elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should be omitted when falsy values are provided")
    {
        VNode* vnode{
            h("div", Data(
                         Attrs{
                             { "href", "null" },
                             { "minlength", "0" },
                             { "value", "false" } }))
        };

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val::null()));
    }

    SECTION("should set truthy values to empty string")
    {
        VNode* vnode{
            h("input", Data(
                           Attrs{
                               { "href", "null" },
                               { "minlength", "0" },
                               { "readonly", "true" } }))
        };

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("readonly")).strictlyEquals(emscripten::val("")));
    }

    SECTION("should be set correctly when namespaced")
    {
        VNode* vnode{
            h("div", Data(
                         Attrs{
                             { "xlink:href", "#foo" } }))
        };

        VDom vdom(getRoot());
        vdom.patch(vnode);

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttributeNS",
                                          emscripten::val("http://www.w3.org/1999/xlink"),
                                          emscripten::val("href")) == emscripten::val("#foo"));
    }
}
