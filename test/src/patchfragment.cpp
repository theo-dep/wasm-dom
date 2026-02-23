#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

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

    SECTION("should patch an element inside a fragment in children")
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
                    span()(std::string("bar"))
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
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("should append elements to fragment")
    {
        VNode vnode1 =
            fragment()(
                span()(std::string("foo"))
            );
        VNode vnode2 =
            fragment()(
                { span()(std::string("foo")),
                  span()(std::string("bar")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("should append elements to fragment in children")
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

    SECTION("should append multiple fragments with elements to fragment")
    {
        VNode vnode1 = div();
        VNode vnode2 =
            div()(
                { fragment()(
                      { span()(std::string("foo")),
                        span()(std::string("bar")) }
                  ),
                  fragment()(
                      { span()(std::string("dog")),
                        span()(std::string("cat")) }
                  ) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["2"]["textContent"], StrictlyEquals(emscripten::val("dog")));
        REQUIRE_THAT(node["children"]["3"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["3"]["textContent"], StrictlyEquals(emscripten::val("cat")));
    }

    SECTION("should patch root element with fragment")
    {
        VNode vnode1 =
            div()(
                span()(std::string("foo"))
            );
        VNode vnode2 =
            fragment()(
                span()(std::string("foo"))
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should patch element with fragment in children")
    {
        VNode vnode1 =
            div()(
                div()(
                    span()(std::string("foo"))
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
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should patch root fragment element with other")
    {
        VNode vnode1 =
            fragment()(
                span()(std::string("foo"))
            );
        VNode vnode2 =
            div()(
                span()(std::string("foo"))
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should patch fragment element with other in children")
    {
        VNode vnode1 =
            div()(
                fragment()(
                    span()(std::string("foo"))
                )
            );
        VNode vnode2 =
            div()(
                div()(
                    span()(std::string("foo"))
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
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should add a fragment with children")
    {
        VNode vnode1 =
            div();
        VNode vnode2 =
            fragment()(
                { span()(std::string("foo")),
                  span()(std::string("bar")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("should add a fragment with children in children")
    {
        VNode vnode1 =
            div();
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
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("should remove a fragment with children")
    {
        VNode vnode1 =
            fragment()(
                { span()(std::string("foo")),
                  span()(std::string("bar")) }
            );
        VNode vnode2 =
            div();
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should remove a fragment with children in children")
    {
        VNode vnode1 =
            div()(
                fragment()(
                    { span()(std::string("foo")),
                      span()(std::string("bar")) }
                )
            );
        VNode vnode2 =
            div();
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
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should remove elements from fragment")
    {
        VNode vnode1 =
            fragment()(
                { span()(std::string("foo")),
                  span()(std::string("bar")) }
            );
        VNode vnode2 =
            fragment()(
                span()(std::string("foo"))
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should remove elements from fragment in children")
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

    SECTION("should update one fragment child with same key but different sel")
    {
        VNode vnode1 =
            fragment()(
                { span(("key", "1"s))(std::string("1")),
                  span(("key", "2"s))(std::string("2")),
                  span(("key", "3"s))(std::string("3")) }
            );
        VNode vnode2 =
            fragment()(
                { span(("key", "1"s))(std::string("1")),
                  i(("key", "2"s))(std::string("2")),
                  span(("key", "3"s))(std::string("3")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("I")));
    }

    SECTION("should patch a fragment node without child")
    {
        VNode vnode1 = fragment();
        VNode vnode2 =
            div()(
                span()(std::string("foo"))
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should patch a fragment node without child in children")
    {
        VNode vnode1 =
            div()(
                fragment()
            );
        VNode vnode2 =
            div()(
                div()(
                    span()(std::string("foo"))
                )
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should update fragment in list")
    {
        auto newElement = GENERATE(span()(std::string("bar")), fragment()(span()(std::string("bar"))));

        SECTION("should append element after fragment node")
        {
            VNode vnode1 =
                div()(
                    fragment()(
                        span()(std::string("foo"))
                    )
                );
            VNode vnode2 =
                div()(
                    { fragment()(
                          span()(std::string("foo"))
                      ),
                      newElement }
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

        SECTION("should append element before fragment node")
        {
            VNode vnode1 =
                div()(
                    fragment()(
                        span()(std::string("foo"))
                    )
                );
            VNode vnode2 =
                div()(
                    { newElement,
                      fragment()(
                          span()(std::string("foo"))
                      ) }
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
            REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("bar")));
            REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        }

        SECTION("should update element after fragment node")
        {
            VNode vnode1 =
                div()(
                    { fragment()(
                          span()(std::string("foo"))
                      ),
                      newElement }
                );
            VNode vnode2 =
                div()(
                    { fragment()(
                          span()(std::string("foo"))
                      ),
                      span()(std::string("fum")) }
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
            REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
            REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
            REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("fum")));
        }

        SECTION("should update element before fragment node")
        {
            VNode vnode1 =
                div()(
                    { newElement,
                      fragment()(
                          span()(std::string("foo"))
                      ) }
                );
            VNode vnode2 =
                div()(
                    { span()(std::string("fum")),
                      fragment()(
                          span()(std::string("foo"))
                      ) }
                );
            VDom vdom(jsDom.root());
            vdom.patch(vnode1);
            emscripten::val node = jsDom.bodyFirstChild();
            REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
            REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
            REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("bar")));
            REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("foo")));
            vdom.patch(vnode2);
            node = jsDom.bodyFirstChild();
            REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
            REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
            REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("fum")));
            REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        }

        SECTION("should remove element after fragment node")
        {
            VNode vnode1 =
                div()(
                    { fragment()(
                          span()(std::string("foo"))
                      ),
                      newElement }
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

        SECTION("should remove element before fragment node")
        {
            VNode vnode1 =
                div()(
                    { newElement,
                      fragment()(
                          span()(std::string("foo"))
                      ) }
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
            REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("bar")));
            REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("foo")));
            vdom.patch(vnode2);
            node = jsDom.bodyFirstChild();
            REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
            REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
            REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
            REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        }
    }

    SECTION("should support empty children")
    {
        wasmdom::Children children;
        VNode vnode1 = fragment()(children);
        children.push_back(span()(std::string("foo")));
        children.push_back(span()(std::string("bar")));
        VNode vnode2 = fragment()(children);
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("should support empty children in children")
    {
        wasmdom::Children children;
        VNode vnode1 = div()(fragment()(children));
        children.push_back(span()(std::string("foo")));
        children.push_back(span()(std::string("bar")));
        VNode vnode2 = div()(fragment()(children));
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
    }

    SECTION("should support adding fragment children in fragment children")
    {
        VNode vnode1 = fragment();
        VNode vnode2 =
            fragment()(
                { div(),
                  div(),
                  div() }
            );
        VNode vnode3 =
            fragment()(
                { div(),
                  div()({ fragment()({ span()(std::string("foo1")),
                                       span()(std::string("bar1")) }),
                          fragment()({ span()(std::string("foo2")),
                                       span()(std::string("bar2")) }) }),
                  div() }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["textContent"], StrictlyEquals(emscripten::val("foo2")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["textContent"], StrictlyEquals(emscripten::val("bar2")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should support updating fragment children in fragment children")
    {
        VNode vnode1 = fragment();
        VNode vnode2 =
            fragment()(
                { div(),
                  div()({ fragment()({ span()(std::string("foo1")),
                                       span()(std::string("bar1")) }),
                          fragment()({ span()(std::string("foo2")),
                                       span()(std::string("bar2")) }) }),
                  div() }
            );
        VNode vnode3 =
            fragment()(
                { div(),
                  div()({ fragment()({ span()(std::string("foo3")),
                                       span()(std::string("bar3")) }),
                          fragment()({ span()(std::string("foo4")),
                                       span()(std::string("bar4")) }) }),
                  div() }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["textContent"], StrictlyEquals(emscripten::val("foo2")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["textContent"], StrictlyEquals(emscripten::val("bar2")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo3")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar3")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["textContent"], StrictlyEquals(emscripten::val("foo4")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["textContent"], StrictlyEquals(emscripten::val("bar4")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should support removing fragment children in fragment children")
    {
        VNode vnode1 = fragment();
        VNode vnode2 =
            fragment()(
                { div(),
                  div()({ fragment()({ span()(std::string("foo1")),
                                       span()(std::string("bar1")) }),
                          fragment()({ span()(std::string("foo2")),
                                       span()(std::string("bar2")) }) }),
                  div() }
            );
        VNode vnode3 =
            fragment()(
                { div(),
                  div(),
                  div() }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["2"]["textContent"], StrictlyEquals(emscripten::val("foo2")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["3"]["textContent"], StrictlyEquals(emscripten::val("bar2")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        node = jsDom.body();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }
}
