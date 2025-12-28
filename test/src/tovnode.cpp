#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "catch_emscripten.hpp"
#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

TEST_CASE("toVNode", "[toVNode]")
{
    const JSDom jsDom;

    SECTION("should convert a node to vnode")
    {
        emscripten::val node = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));

        VNode vnode{ VNode::toVNode(node) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should convert text node to vnode")
    {
        emscripten::val node = jsDom.document().call<emscripten::val>("createTextNode", emscripten::val("Hello world!"));

        VNode vnode{ VNode::toVNode(node) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("#text")));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("Hello world!")));
    }

    SECTION("should convert comment node to vnode")
    {
        emscripten::val node = jsDom.document().call<emscripten::val>("createComment", emscripten::val("Hello world!"));

        VNode vnode{ VNode::toVNode(node) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("#comment")));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("Hello world!")));
    }

    SECTION("should convert a node with attributes to vnode")
    {
        emscripten::val node = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        node.call<void>("setAttribute", emscripten::val("src"), emscripten::val("http://localhost/"));
        node.call<void>("setAttribute", emscripten::val("data-foo"), emscripten::val("bar"));
        node.call<void>("setAttribute", emscripten::val("data-bar"), emscripten::val("foo"));

        VNode vnode{ VNode::toVNode(node) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("src")), StrictlyEquals(emscripten::val("http://localhost/")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")), StrictlyEquals(emscripten::val("bar")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-bar")), StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should convert a node with children to vnode")
    {
        emscripten::val parent = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        emscripten::val h1 = jsDom.document().call<emscripten::val>("createElement", emscripten::val("h1"));
        h1.call<void>("appendChild", jsDom.document().call<emscripten::val>("createTextNode", emscripten::val("Hello World!")));
        emscripten::val p = jsDom.document().call<emscripten::val>("createElement", emscripten::val("p"));
        p.call<void>("appendChild", jsDom.document().call<emscripten::val>("createTextNode", emscripten::val("foo")));
        p.call<void>("appendChild", jsDom.document().call<emscripten::val>("createComment", emscripten::val("bar")));
        emscripten::val child = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        parent.call<void>("appendChild", h1);
        parent.call<void>("appendChild", p);
        parent.call<void>("appendChild", child);

        VNode vnode{ VNode::toVNode(parent) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("H1")));
        REQUIRE_THAT(node["childNodes"]["0"]["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["childNodes"]["0"]["nodeName"], StrictlyEquals(emscripten::val("#text")));
        REQUIRE_THAT(node["childNodes"]["0"]["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Hello World!")));
        REQUIRE_THAT(node["childNodes"]["1"]["tagName"], StrictlyEquals(emscripten::val("P")));
        REQUIRE_THAT(node["childNodes"]["1"]["childNodes"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["childNodes"]["1"]["childNodes"]["0"]["nodeName"], StrictlyEquals(emscripten::val("#text")));
        REQUIRE_THAT(node["childNodes"]["1"]["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["childNodes"]["1"]["childNodes"]["1"]["nodeName"], StrictlyEquals(emscripten::val("#comment")));
        REQUIRE_THAT(node["childNodes"]["1"]["childNodes"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar")));
        REQUIRE_THAT(node["childNodes"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should convert a node with attributes and children to vnode")
    {
        emscripten::val parent = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        parent.call<void>("setAttribute", emscripten::val("data-foo"), emscripten::val("foo"));
        emscripten::val img = jsDom.document().call<emscripten::val>("createElement", emscripten::val("img"));
        img.call<void>("setAttribute", emscripten::val("src"), emscripten::val("http://localhost/"));
        emscripten::val div = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        parent.call<void>("appendChild", img);
        parent.call<void>("appendChild", div);

        VNode vnode{ VNode::toVNode(parent) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        emscripten::val node = jsDom.bodyFirstChild();

        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")), StrictlyEquals(emscripten::val("foo")));
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("IMG")));
        REQUIRE_THAT(node["childNodes"]["0"].call<emscripten::val>("getAttribute", emscripten::val("src")), StrictlyEquals(emscripten::val("http://localhost/")));
        REQUIRE_THAT(node["childNodes"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should remove previous children of the root element")
    {
        emscripten::val h2 = jsDom.document().call<emscripten::val>("createElement", emscripten::val("h2"));
        h2.set("textContent", emscripten::val("Hello"));
        emscripten::val prevNode = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        prevNode.set("id", emscripten::val("id"));
        prevNode.set("className", emscripten::val("class"));
        prevNode.call<void>("appendChild", h2);

        VNode nextVNode =
            div(("id", "id"s),
                ("class", "class"s))(
                { span()(
                    std::string("Hi")
                ) }
            );

        VDom vdom(prevNode);
        vdom.patch(nextVNode);

        emscripten::val node = nextVNode.node();

        REQUIRE_THAT(node, StrictlyEquals(prevNode));
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["id"], StrictlyEquals(emscripten::val("id")));
        REQUIRE_THAT(node["className"], StrictlyEquals(emscripten::val("class")));
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Hi")));
    }

    SECTION("should support patching in a DocumentFragment")
    {
        emscripten::val prevNode = jsDom.document().call<emscripten::val>("createDocumentFragment");

        VNode nextVNode =
            fragment()(
                { div(("id", "id"s), ("class", "class"s))(
                    { span()(
                        std::string("Hi")
                    ) }
                ) }
            );

        VDom vdom(prevNode);
        vdom.patch(nextVNode);

        emscripten::val node = nextVNode.node();

        REQUIRE_THAT(node, StrictlyEquals(prevNode));
        REQUIRE_THAT(node["nodeType"], StrictlyEquals(emscripten::val(11)));
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["childNodes"]["0"]["id"], StrictlyEquals(emscripten::val("id")));
        REQUIRE_THAT(node["childNodes"]["0"]["className"], StrictlyEquals(emscripten::val("class")));
        REQUIRE_THAT(node["childNodes"]["0"]["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["childNodes"]["0"]["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Hi")));
    }

    SECTION("should remove some children of the root element")
    {
        emscripten::val h2 = jsDom.document().call<emscripten::val>("createElement", emscripten::val("h2"));
        h2.set("textContent", emscripten::val("Hello"));
        emscripten::val prevNode = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        prevNode.set("id", emscripten::val("id"));
        prevNode.set("className", emscripten::val("class"));
        emscripten::val text = jsDom.document().call<emscripten::val>("createTextNode", emscripten::val("Foobar"));
        text.set("testProperty", emscripten::val(123));
        prevNode.call<void>("appendChild", text);
        prevNode.call<void>("appendChild", h2);

        VNode nextVNode =
            div(("id", "id"s),
                ("class", "class"s))(
                { t("Foobar") }
            );

        VDom vdom(prevNode);
        vdom.patch(nextVNode);

        emscripten::val node = nextVNode.node();

        REQUIRE_THAT(node, StrictlyEquals(prevNode));
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["id"], StrictlyEquals(emscripten::val("id")));
        REQUIRE_THAT(node["className"], StrictlyEquals(emscripten::val("class")));
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["nodeType"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["childNodes"]["0"]["wholeText"], StrictlyEquals(emscripten::val("Foobar")));
        REQUIRE_THAT(node["childNodes"]["0"]["testProperty"], StrictlyEquals(emscripten::val(123)));
    }

    SECTION("should remove text elements")
    {
        emscripten::val valH2 = jsDom.document().call<emscripten::val>("createElement", emscripten::val("h2"));
        valH2.set("textContent", emscripten::val("Hello"));
        emscripten::val prevNode = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        prevNode.set("id", emscripten::val("id"));
        prevNode.set("className", emscripten::val("class"));
        emscripten::val text = jsDom.document().call<emscripten::val>("createTextNode", emscripten::val("Foobar"));
        prevNode.call<void>("appendChild", text);
        prevNode.call<void>("appendChild", valH2);

        VNode nextVNode =
            div(("id", "id"s),
                ("class", "class"s))(
                { h2()(
                    std::string("Hello")
                ) }
            );

        VDom vdom(prevNode);
        vdom.patch(nextVNode);

        emscripten::val node = nextVNode.node();

        REQUIRE_THAT(node, StrictlyEquals(prevNode));
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["id"], StrictlyEquals(emscripten::val("id")));
        REQUIRE_THAT(node["className"], StrictlyEquals(emscripten::val("class")));
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["nodeType"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Hello")));
    }
}
