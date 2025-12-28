#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "catch_emscripten.hpp"
#include "jsdom.hpp"

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
        REQUIRE_THAT(node["nodeType"], StrictlyEquals(jsDom.document()["TEXT_NODE"]));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("foo")));
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
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("bar")));
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
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
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
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }
}
