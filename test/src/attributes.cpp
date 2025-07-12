#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;

TEST_CASE("attributes", "[attributes]")
{
    setupDom();

    SECTION("should have their provided values")
    {
        ScopedVNode vnode{
            h("div", Data(
                         Attrs{
                             { "href", "/foo" },
                             { "minlength", "1" },
                             { "value", "foo" } }))
        };

        patch(getRoot(), vnode.get());

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
        ScopedVNode vnode{ h("div", data) };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));

        ScopedVNode vnode2{ h("div", data) };

        patch(vnode.release(), vnode2.get());

        elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("/foo")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("1")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should be omitted when falsy values are provided")
    {
        ScopedVNode vnode{
            h("div", Data(
                         Attrs{
                             { "href", "null" },
                             { "minlength", "0" },
                             { "value", "false" } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("value")).strictlyEquals(emscripten::val::null()));
    }

    SECTION("should set truthy values to empty string")
    {
        ScopedVNode vnode{
            h("input", Data(
                           Attrs{
                               { "href", "null" },
                               { "minlength", "0" },
                               { "readonly", "true" } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("href")).strictlyEquals(emscripten::val("null")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("minlength")).strictlyEquals(emscripten::val("0")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("readonly")).strictlyEquals(emscripten::val("")));
    }

    SECTION("should be set correctly when namespaced")
    {
        ScopedVNode vnode{
            h("div", Data(
                         Attrs{
                             { "xlink:href", "#foo" } }))
        };

        patch(getRoot(), vnode.get());

        emscripten::val elm = getBodyFirstChild();

        REQUIRE(elm.call<emscripten::val>("getAttributeNS",
                                          emscripten::val("http://www.w3.org/1999/xlink"),
                                          emscripten::val("href")) == emscripten::val("#foo"));
    }
}
