#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "jsdom.hpp"

#include <emscripten.h>

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("patchFragment", "[patchFragment]")
{
    const JSDom jsDom;

    SECTION("should create fragments")
    {
        VNode vnode =
            fragment()(
                t("foo")
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["nodeType"].strictlyEquals(jsDom.document()["TEXT_NODE"]));
        REQUIRE(node["textContent"].strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should patch an element inside a fragment")
    {
        VNode vnode1 =
            fragment()(
                span()(std::string("foo"))
            );
        VNode vnode2 =
            fragment()(
                span()(std::string("bar"))
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["textContent"].strictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["textContent"].strictlyEquals(emscripten::val("bar")));
    }

    SECTION("should append elements to fragment")
    {
        VNode vnode1 =
            div()(
                fragment()(
                    span()(std::string("foo"))
                )
            );
        VNode vnode2 =
            div()(
                fragment()(
                    { span()(std::string("foo")),
                      span()(std::string("bar")) }
                )
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(node["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(node["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["children"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
    }

    SECTION("should remove elements from fragment")
    {
        VNode vnode1 =
            div()(
                fragment()(
                    { span()(std::string("foo")),
                      span()(std::string("bar")) }
                )
            );
        VNode vnode2 =
            div()(
                fragment()(
                    span()(std::string("foo"))
                )
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(node["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(node["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["children"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
    }
}
