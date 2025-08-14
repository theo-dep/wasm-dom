#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

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

        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should convert text node to vnode")
    {
        emscripten::val node = jsDom.document().call<emscripten::val>("createTextNode", emscripten::val("Hello world!"));

        VNode vnode{ VNode::toVNode(node) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        node = jsDom.bodyFirstChild();

        REQUIRE(node["nodeName"].strictlyEquals(emscripten::val("#text")));
        REQUIRE(node["textContent"].strictlyEquals(emscripten::val("Hello world!")));
    }

    SECTION("should convert comment node to vnode")
    {
        emscripten::val node = jsDom.document().call<emscripten::val>("createComment", emscripten::val("Hello world!"));

        VNode vnode{ VNode::toVNode(node) };

        VDom vdom(jsDom.root());
        vdom.patch(vnode);

        node = jsDom.bodyFirstChild();

        REQUIRE(node["nodeName"].strictlyEquals(emscripten::val("#comment")));
        REQUIRE(node["textContent"].strictlyEquals(emscripten::val("Hello world!")));
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

        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("src")).strictlyEquals(emscripten::val("http://localhost/")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("bar")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("data-bar")).strictlyEquals(emscripten::val("foo")));
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

        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["childNodes"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(node["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("H1")));
        REQUIRE(node["childNodes"]["0"]["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["childNodes"]["0"]["nodeName"].strictlyEquals(emscripten::val("#text")));
        REQUIRE(node["childNodes"]["0"]["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Hello World!")));
        REQUIRE(node["childNodes"]["1"]["tagName"].strictlyEquals(emscripten::val("P")));
        REQUIRE(node["childNodes"]["1"]["childNodes"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(node["childNodes"]["1"]["childNodes"]["0"]["nodeName"].strictlyEquals(emscripten::val("#text")));
        REQUIRE(node["childNodes"]["1"]["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(node["childNodes"]["1"]["childNodes"]["1"]["nodeName"].strictlyEquals(emscripten::val("#comment")));
        REQUIRE(node["childNodes"]["1"]["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
        REQUIRE(node["childNodes"]["2"]["tagName"].strictlyEquals(emscripten::val("DIV")));
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

        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("foo")));
        REQUIRE(node["childNodes"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(node["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("IMG")));
        REQUIRE(node["childNodes"]["0"].call<emscripten::val>("getAttribute", emscripten::val("src")).strictlyEquals(emscripten::val("http://localhost/")));
        REQUIRE(node["childNodes"]["1"]["tagName"].strictlyEquals(emscripten::val("DIV")));
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

        REQUIRE(node.strictlyEquals(prevNode));
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["id"].strictlyEquals(emscripten::val("id")));
        REQUIRE(node["className"].strictlyEquals(emscripten::val("class")));
        REQUIRE(node["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Hi")));
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

        REQUIRE(node.strictlyEquals(prevNode));
        REQUIRE(node["nodeType"].strictlyEquals(emscripten::val(11)));
        REQUIRE(node["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["childNodes"]["0"]["id"].strictlyEquals(emscripten::val("id")));
        REQUIRE(node["childNodes"]["0"]["className"].strictlyEquals(emscripten::val("class")));
        REQUIRE(node["childNodes"]["0"]["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(node["childNodes"]["0"]["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Hi")));
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

        REQUIRE(node.strictlyEquals(prevNode));
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["id"].strictlyEquals(emscripten::val("id")));
        REQUIRE(node["className"].strictlyEquals(emscripten::val("class")));
        REQUIRE(node["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["nodeType"].strictlyEquals(emscripten::val(3)));
        REQUIRE(node["childNodes"]["0"]["wholeText"].strictlyEquals(emscripten::val("Foobar")));
        REQUIRE(node["childNodes"]["0"]["testProperty"].strictlyEquals(emscripten::val(123)));
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

        REQUIRE(node.strictlyEquals(prevNode));
        REQUIRE(node["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(node["id"].strictlyEquals(emscripten::val("id")));
        REQUIRE(node["className"].strictlyEquals(emscripten::val("class")));
        REQUIRE(node["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["nodeType"].strictlyEquals(emscripten::val(1)));
        REQUIRE(node["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Hello")));
    }
}
