#include <catch2/catch_test_macros.hpp>

#include "internals/domkeys.hpp"
#include "wasm-dom.hpp"

#include "catch_emscripten.hpp"
#include "jsdom.hpp"

using namespace wasmdom;
using namespace wasmdom::internals;
using namespace wasmdom::dsl;

inline bool onClick(emscripten::val /*event*/)
{
    return true;
}

VNode spanNum(int i)
{
    return span(("key", std::to_string(i)))(
        std::to_string(i)
    );
}

VNode spanNumWithOpacity(int z, std::string o)
{
    std::string zString = std::to_string(z);
    std::string opacity = std::string("opacity: ");
    opacity.append(o);
    return span(("key", zString), ("style", opacity))(
        zString
    );
}

std::vector<int> shuffle(std::vector<int>& arr, int nodes)
{
    std::vector<int> newArr;
    newArr.resize(nodes);
    for (int n = 0; n < nodes; ++n) {
        newArr[n] = arr[n];
    }
    for (int n = 0; n < nodes; ++n) {
        int i = rand() % nodes;

        int temp = newArr[n];
        newArr[n] = newArr[i];
        newArr[i] = temp;
    }
    return newArr;
}

TEST_CASE("patch", "[patch]")
{
    const JSDom jsDom;

    SECTION("should first handle nullptr VNode")
    {
        VNode vnode = span();
        VDom vdom;
        vdom.patch(vnode);
    }

    SECTION("should handle nullptr VNode")
    {
        REQUIRE_THAT(jsDom.document()["body"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(jsDom.document()["body"]["firstChild"], StrictlyEquals(emscripten::val(jsDom.root())));
        VNode vnode = nullptr;
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(jsDom.document()["body"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(jsDom.document()["body"]["firstChild"], StrictlyEquals(emscripten::val(jsDom.root())));
    }

    SECTION("should patch a node")
    {
        REQUIRE_THAT(jsDom.document()["body"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(jsDom.document()["body"]["firstChild"], StrictlyEquals(emscripten::val(jsDom.root())));
        VNode vnode = span();
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(jsDom.document()["body"]["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["className"], StrictlyEquals(emscripten::val("")));
    }

    SECTION("should patch the same node")
    {
        VNode vnode = div();
        VDom vdom(jsDom.root());
        VNode nodePtr = vdom.patch(vnode);
        vdom.patch(nodePtr);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should have a tag")
    {
        VNode vnode = div();
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should have the correct namespace")
    {
        std::string svgNamespace = "http://www.w3.org/2000/svg";
        VNode vnode =
            div()(
                div(("ns", svgNamespace))
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["firstChild"]["namespaceURI"], StrictlyEquals(emscripten::val(svgNamespace)));
    }

    SECTION("should inject svg namespace")
    {
        std::string svgNamespace = "http://www.w3.org/2000/svg";
        std::string XHTMLNamespace = "http://www.w3.org/1999/xhtml";
        VNode vnode =
            svg()(
                foreignObject()(
                    div()(
                        t("I am HTML embedded in SVG")
                    )
                )
            );

        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["namespaceURI"], StrictlyEquals(emscripten::val(svgNamespace)));
        REQUIRE_THAT(node["firstChild"]["namespaceURI"], StrictlyEquals(emscripten::val(svgNamespace)));
        REQUIRE_THAT(node["firstChild"]["firstChild"]["namespaceURI"], StrictlyEquals(emscripten::val(XHTMLNamespace)));
    }

    SECTION("should create elements with class")
    {
        VNode vnode = div(("class", "foo"s));
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("class")), StrictlyEquals(emscripten::val("foo")));
    }

    SECTION("should create elements with text content")
    {
        VNode vnode =
            div()(
                t("I am a string")
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["innerHTML"], StrictlyEquals(emscripten::val("I am a string")));
    }

    // TODO : how can we test this?
    SECTION("should create elements with text content in utf8") {}

    SECTION("should create elements with span and text content")
    {
        VNode vnode =
            a()(
                { span(),
                  t("I am a string") }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["childNodes"]["1"]["textContent"], StrictlyEquals(emscripten::val("I am a string")));
    }

    SECTION("is a patch of the root element")
    {
        emscripten::val nodeWithIdAndClass = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
        nodeWithIdAndClass.set("id", emscripten::val("id"));
        nodeWithIdAndClass.set("className", emscripten::val("class"));
        VNode vnode =
            div(("id", "id"s),
                ("class", "class"s))(
                span()(std::string("Hi"))
            );

        VDom vdom(nodeWithIdAndClass);
        vdom.patch(vnode);
        emscripten::val node = vnode.node();
        REQUIRE_THAT(node, StrictlyEquals(nodeWithIdAndClass));
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["id"], StrictlyEquals(emscripten::val("id")));
        REQUIRE_THAT(node["className"], StrictlyEquals(emscripten::val("class")));
    }

    SECTION("should create comments")
    {
        VNode vnode = comment(std::string("test"));
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["nodeType"], StrictlyEquals(jsDom.document()["COMMENT_NODE"]));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("test")));
    }

    // TODO
    SECTION("should create an element created inside an iframe") {}

    SECTION("should append elements")
    {
        VNode vnode1 =
            span()(
                spanNum(1)
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
    }

    SECTION("should prepend elements")
    {
        VNode vnode1 =
            span()(
                { spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
    }

    SECTION("should add elements in the middle")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
    }

    SECTION("should add elements at begin and end")
    {
        VNode vnode1 =
            span()(
                { spanNum(2),
                  spanNum(3),
                  spanNum(4) }
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
    }

    SECTION("should add children to parent with no children")
    {
        VNode vnode1 = span(("key", "span"s));
        VNode vnode2 =
            span(("key", "span"s))(
                { span()(std::string("1")),
                  span()(std::string("2")),
                  span()(std::string("3")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
    }

    SECTION("should remove all children from parent")
    {
        VNode vnode1 =
            span(("key", "span"s))(
                { span()(std::string("1")),
                  span()(std::string("2")),
                  span()(std::string("3")) }
            );
        VNode vnode2 = span(("key", "span"s));
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should update one child with same key but different sel")
    {
        VNode vnode1 =
            span(("key", "span"s))(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3) }
            );
        VNode vnode2 =
            span(("key", "span"s))(
                { spanNum(1),
                  i(("key", "2"s))(
                      std::string("2")
                  ),
                  spanNum(3) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("I")));
    }

    SECTION("should remove elements from the beginning")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
    }

    SECTION("should remove elements from the end")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
    }

    SECTION("should remove elements from the middle")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(4),
                  spanNum(5) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
    }

    SECTION("should move element forward")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4) }
            );
        VNode vnode2 =
            span()(
                { spanNum(2),
                  spanNum(3),
                  spanNum(1),
                  spanNum(4) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
    }

    SECTION("should move element to end")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3) }
            );
        VNode vnode2 =
            span()(
                { spanNum(2),
                  spanNum(3),
                  spanNum(1) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
    }

    SECTION("should move element backwards")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4) }
            );
        VNode vnode2 =
            span()(
                { spanNum(1),
                  spanNum(4),
                  spanNum(2),
                  spanNum(3) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
    }

    SECTION("should swap first and last")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4) }
            );
        VNode vnode2 =
            span()(
                { spanNum(4),
                  spanNum(2),
                  spanNum(3),
                  spanNum(1) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
    }

    SECTION("should move to left and replace")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(4),
                  spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(6) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(5)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("6")));
    }

    SECTION("should move to left and leaves hole")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(4),
                  spanNum(6) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("6")));
    }

    SECTION("should handle moved and set to undefined element ending at the end")
    {
        VNode vnode1 =
            span()(
                { spanNum(2),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(4),
                  spanNum(5),
                  spanNum(3) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
    }

    SECTION("should move a key in non-keyed nodes with a size up")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  span()(std::string("a")),
                  span()(std::string("b")),
                  span()(std::string("c")) }
            );
        VNode vnode2 =
            span()(
                { span()(std::string("d")),
                  span()(std::string("a")),
                  span()(std::string("b")),
                  span()(std::string("c")),
                  spanNum(1),
                  span()(std::string("e")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(4)));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("1abc")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(6)));
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("dabc1e")));
    }

    SECTION("should reverse elements")
    {
        VNode vnode1 =
            span()(
                { spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5),
                  spanNum(6),
                  spanNum(7),
                  spanNum(8) }
            );
        VNode vnode2 =
            span()(
                { spanNum(8),
                  spanNum(7),
                  spanNum(6),
                  spanNum(5),
                  spanNum(4),
                  spanNum(3),
                  spanNum(2),
                  spanNum(1) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(8)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(8)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("8")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("7")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("6")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["5"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["6"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["7"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
    }

    SECTION("should reverse elements with 0")
    {
        VNode vnode1 =
            span()(
                { spanNum(0),
                  spanNum(1),
                  spanNum(2),
                  spanNum(3),
                  spanNum(4),
                  spanNum(5) }
            );
        VNode vnode2 =
            span()(
                { spanNum(4),
                  spanNum(3),
                  spanNum(2),
                  spanNum(1),
                  spanNum(5),
                  spanNum(0) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(6)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(6)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
        REQUIRE_THAT(node["children"]["5"]["innerHTML"], StrictlyEquals(emscripten::val("0")));
    }

    SECTION("should handle random shuffles")
    {
        int n;
        int i;
        std::vector<int> arr;
        std::vector<std::string> opacities;
        int nodes = 14;
        int samples = 5;

        arr.resize(nodes);
        opacities.resize(nodes);
        for (n = 0; n < nodes; ++n) {
            arr[n] = n;
        }

        for (n = 0; n < samples; ++n) {
            Children children;
            for (i = 0; i < nodes; ++i) {
                children.push_back(spanNumWithOpacity(arr[i], std::string("1")));
            }
            VNode vnode1 = span()(children);

            std::vector<int> shufArr = shuffle(arr, nodes);

            emscripten::val node = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
            VDom vdom(node);
            vdom.patch(vnode1);
            node = vnode1.node();
            for (i = 0; i < nodes; ++i) {
                REQUIRE_THAT(node["children"][std::to_string(i)]["innerHTML"], StrictlyEquals(emscripten::val(std::to_string(i))));
                opacities[i] = std::string("0.");
                opacities[i].append(std::to_string(rand() % 99999));
            }
            Children opacityChildren;
            for (i = 0; i < nodes; ++i) {
                opacityChildren.push_back(spanNumWithOpacity(shufArr[i], opacities[i]));
            }
            VNode vnode2 = span()(opacityChildren);

            vdom.patch(vnode2);
            node = vnode2.node();
            for (i = 0; i < nodes; ++i) {
                REQUIRE_THAT(node["children"][std::to_string(i)]["innerHTML"], StrictlyEquals(emscripten::val(std::to_string(shufArr[i]))));
                REQUIRE_THAT(emscripten::val(opacities[i]).call<emscripten::val>("indexOf", node["children"][std::to_string(i)]["style"]["opacity"]), StrictlyEquals(emscripten::val(0)));
            }
        }
    }

    SECTION("should support null/undefined children")
    {
        VNode vnode1 =
            span()(
                { span()(std::string("0")),
                  span()(std::string("1")),
                  span()(std::string("2")),
                  span()(std::string("3")),
                  span()(std::string("4")),
                  span()(std::string("5")) }
            );
        VNode vnode2 =
            span()(
                { nullptr,
                  span()(std::string("2")),
                  nullptr,
                  nullptr,
                  span()(std::string("1")),
                  span()(std::string("0")),
                  nullptr,
                  span()(std::string("5")),
                  span()(std::string("4")),
                  nullptr,
                  span()(std::string("3")),
                  nullptr }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(6)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(6)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("0")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["5"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
    }

    SECTION("should support all null/undefined children")
    {
        VNode vnode1 =
            span()(
                { span()(std::string("0")),
                  span()(std::string("1")),
                  span()(std::string("2")),
                  span()(std::string("3")),
                  span()(std::string("4")),
                  span()(std::string("5")) }
            );
        VNode vnode2 =
            span()(
                { nullptr,
                  nullptr,
                  nullptr,
                  nullptr,
                  nullptr,
                  nullptr }
            );
        VNode vnode3 =
            span()(
                { span()(std::string("5")),
                  span()(std::string("4")),
                  span()(std::string("3")),
                  span()(std::string("2")),
                  span()(std::string("1")),
                  span()(std::string("0")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        vdom.patch(vnode2);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(6)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("5")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("4")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("3")));
        REQUIRE_THAT(node["children"]["3"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["4"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["5"]["innerHTML"], StrictlyEquals(emscripten::val("0")));
    }

    SECTION("should handle random shuffles with null/undefined children")
    {
        int i;
        int j;
        int r;
        int len;
        std::vector<int> arr;
        int maxArrLen = 15;
        int samples = 5;

        for (i = 0; i < samples; ++i) {
            len = rand() % (maxArrLen + 1);
            arr = std::vector<int>();
            arr.resize(len);
            for (j = 0; j < len; ++j) {
                r = rand() % 100;
                if (r < 50)
                    arr[j] = j;
                else
                    arr[j] = 0;
            }
            std::vector<int> shufArr = shuffle(arr, len);

            emscripten::val node = jsDom.document().call<emscripten::val>("createElement", emscripten::val("div"));
            VDom vdom(node);
            vdom.patch(div());

            Children children = Children();
            for (j = 0; j < len; ++j) {
                children.push_back(shufArr[j] == 0 ? nullptr : span()(std::to_string(shufArr[j])));
            }
            VNode vnode = div()(children);
            vdom.patch(vnode);
            node = vnode.node();
            r = 0;
            for (j = 0; j < len; ++j) {
                if (shufArr[j] != 0) {
                    REQUIRE_THAT(node["children"][std::to_string(r)]["innerHTML"], StrictlyEquals(emscripten::val(std::to_string(shufArr[j]))));
                    ++r;
                }
            }
            REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(r)));
        }
    }

    SECTION("should append elements")
    {
        VNode vnode1 =
            span()(
                { span()(std::string("Hello")) }
            );
        VNode vnode2 =
            span()(
                { span()(std::string("Hello")),
                  span()(std::string("World")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("Hello")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("Hello")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("World")));
    }

    SECTION("should handle unmoved text nodes")
    {
        VNode vnode1 =
            div()(
                { t("Text"),
                  span()(std::string("Span")) }
            );
        VNode vnode2 =
            div()(
                { t("Text"),
                  span()(std::string("Span")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text")));
    }

    SECTION("should handle changing text children")
    {
        VNode vnode1 =
            div()(
                { t("Text"),
                  span()(std::string("Span")) }
            );
        VNode vnode2 =
            div()(
                { t("Text2"),
                  span()(std::string("Span")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text2")));
    }

    SECTION("should handle unmoved comment nodes")
    {
        VNode vnode1 =
            div()(
                { comment(std::string("Text")),
                  span()(std::string("Span")) }
            );
        VNode vnode2 =
            div()(
                { comment(std::string("Text")),
                  span()(std::string("Span")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text")));
    }

    SECTION("should handle changing comment text")
    {
        VNode vnode1 =
            div()(
                { comment(std::string("Text")),
                  span()(std::string("Span")) }
            );
        VNode vnode2 =
            div()(
                { comment(std::string("Text2")),
                  span()(std::string("Span")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Text2")));
    }

    SECTION("should handle changing empty comment")
    {
        VNode vnode1 =
            div()(
                { comment(),
                  span()(std::string("Span")) }
            );
        VNode vnode2 =
            div()(
                { comment(std::string("Test")),
                  span()(std::string("Span")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Test")));
    }

    SECTION("should prepend element")
    {
        VNode vnode1 =
            span()(
                { span()(std::string("World")) }
            );
        VNode vnode2 =
            span()(
                { span()(std::string("Hello")),
                  span()(std::string("World")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("World")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("Hello")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("World")));
    }

    SECTION("should prepend element of different tag type")
    {
        VNode vnode1 =
            span()(
                { span()(std::string("World")) }
            );
        VNode vnode2 =
            span()(
                { div()(std::string("Hello")),
                  span()(std::string("World")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("World")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("Hello")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("World")));
    }

    SECTION("should remove elements")
    {
        VNode vnode1 =
            div()(
                { span()(std::string("One")),
                  span()(std::string("Two")),
                  span()(std::string("Three")) }
            );
        VNode vnode2 =
            div()(
                { span()(std::string("One")),
                  span()(std::string("Three")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("One")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("Two")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("Three")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("One")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("Three")));
    }

    SECTION("should remove a single text node")
    {
        VNode vnode1 = div()(std::string("One"));
        VNode vnode2 = div();
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("One")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("")));
    }

    SECTION("should remove a single text node when children are updated")
    {
        VNode vnode1 = div()(std::string("One"));
        VNode vnode2 =
            div()(
                { div()(std::string("Two")),
                  span()(std::string("Three")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["textContent"], StrictlyEquals(emscripten::val("One")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Two")));
        REQUIRE_THAT(node["childNodes"]["1"]["textContent"], StrictlyEquals(emscripten::val("Three")));
    }

    SECTION("should remove a text node among other elements")
    {
        VNode vnode1 =
            div()(
                { t(std::string("One")),
                  span()(std::string("Two")) }
            );
        VNode vnode2 =
            div()(
                { div()(std::string("Three")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("One")));
        REQUIRE_THAT(node["childNodes"]["1"]["textContent"], StrictlyEquals(emscripten::val("Two")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["childNodes"]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node["childNodes"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["childNodes"]["0"]["textContent"], StrictlyEquals(emscripten::val("Three")));
    }

    SECTION("should reorder elements")
    {
        VNode vnode1 =
            span()(
                { span()(std::string("One")),
                  div()(std::string("Two")),
                  b()(std::string("Three")) }
            );
        VNode vnode2 =
            span()(
                { b()(std::string("Three")),
                  span()(std::string("One")),
                  div()(std::string("Two")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("One")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("Two")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("Three")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("Three")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("One")));
        REQUIRE_THAT(node["children"]["2"]["innerHTML"], StrictlyEquals(emscripten::val("Two")));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("B")));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should support null/undefined children")
    {
        VNode vnode1 =
            i()(
                { nullptr,
                  i()(std::string("1")),
                  i()(std::string("2")),
                  nullptr }
            );
        VNode vnode2 =
            i()(
                { i()(std::string("2")),
                  nullptr,
                  nullptr,
                  i()(std::string("1")),
                  nullptr }
            );
        VNode vnode3 =
            i()(
                { nullptr,
                  i()(std::string("1")),
                  nullptr,
                  nullptr,
                  i()(std::string("2")),
                  nullptr,
                  nullptr }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        vdom.patch(vnode3);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
    }

    SECTION("should support all null/undefined children")
    {
        VNode vnode1 =
            i()(
                { i()(std::string("1")),
                  i()(std::string("2")) }
            );
        VNode vnode2 =
            i()(
                { nullptr,
                  nullptr }
            );
        VNode vnode3 =
            i()(
                { i()(std::string("2")),
                  i()(std::string("1")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        vdom.patch(vnode2);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["children"]["0"]["innerHTML"], StrictlyEquals(emscripten::val("2")));
        REQUIRE_THAT(node["children"]["1"]["innerHTML"], StrictlyEquals(emscripten::val("1")));
    }

    SECTION("should set asmDomRaws")
    {
        VNode vnode1 = i(("foo", emscripten::val("")));
        VNode vnode2 = i(("bar", emscripten::val("")));
        VNode vnode3 = i();
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node[nodeRawsKey]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node[nodeRawsKey]["0"], StrictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        REQUIRE_THAT(node[nodeRawsKey]["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(node[nodeRawsKey]["0"], StrictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode3);
        REQUIRE_THAT(node[nodeRawsKey]["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should set asmDomEvents")
    {
        VNode vnode1 = i(("onclick", f(onClick)));
        VNode vnode2 = i(("onkeydown", f(onClick)));
        VNode vnode3 = i();
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", node[nodeEventsKey]);
        REQUIRE_THAT(keys["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(keys["0"], StrictlyEquals(emscripten::val("click")));
        vdom.patch(vnode2);
        keys = emscripten::val::global("Object").call<emscripten::val>("keys", node[nodeEventsKey]);
        REQUIRE_THAT(keys["length"], StrictlyEquals(emscripten::val(1)));
        REQUIRE_THAT(keys["0"], StrictlyEquals(emscripten::val("keydown")));
        vdom.patch(vnode3);
        keys = emscripten::val::global("Object").call<emscripten::val>("keys", node[nodeEventsKey]);
        REQUIRE_THAT(keys["length"], StrictlyEquals(emscripten::val(0)));
    }

    SECTION("should patch a WebComponent")
    {
        VNode vnode = webComponent();
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("WEB-COMPONENT")));
    }

    SECTION("should patch a WebComponent with attributes")
    {
        VNode vnode = webComponent(("foo", "bar"s), ("bar", "42"s));
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("WEB-COMPONENT")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("foo")), StrictlyEquals(emscripten::val("bar")));
        REQUIRE_THAT(node.call<emscripten::val>("getAttribute", emscripten::val("bar")), StrictlyEquals(emscripten::val("42")));
    }

    SECTION("should patch a WebComponent with eventListeners")
    {
        VNode vnode =
            webComponent(("onclick", f(onClick)), ("onfoo-event", f(onClick)));
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["nodeName"], StrictlyEquals(emscripten::val("WEB-COMPONENT")));
    }

    SECTION("should create a template node")
    {
        VNode vnode =
            hTemplate(("id", "template-node"s))(
                { style()("p { color : green; }"),
                  p()(std::string("Hello world!")) }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode);
        emscripten::val tmpl = jsDom.document().call<emscripten::val>("getElementById", emscripten::val("template-node"));
        emscripten::val fragment = tmpl["content"].call<emscripten::val>("cloneNode", emscripten::val(true));
        REQUIRE_THAT(fragment["nodeName"], StrictlyEquals(emscripten::val("#document-fragment")));
    }

    SECTION("should support empty children")
    {
        wasmdom::Children children;
        VNode vnode1 = div()(children);
        children.push_back(span()(std::string("foo")));
        children.push_back(span()(std::string("bar")));
        VNode vnode2 = div()(children);
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

    SECTION("should support adding children in children")
    {
        VNode vnode1 = div();
        VNode vnode2 =
            div()(
                { div(),
                  div(),
                  div() }
            );
        VNode vnode3 =
            div()(
                { div(),
                  div()({ div()({ span()(std::string("foo1")),
                                  span()(std::string("bar1")) }),
                          div()({ span()(std::string("foo2")),
                                  span()(std::string("bar2")) }) }),
                  div() }
            );
        VDom vdom(jsDom.root());
        vdom.patch(vnode1);
        emscripten::val node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        node = jsDom.bodyFirstChild();
        REQUIRE_THAT(node["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["length"], StrictlyEquals(emscripten::val(3)));
        REQUIRE_THAT(node["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
        REQUIRE_THAT(node["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["0"]["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar1")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["children"]["length"], StrictlyEquals(emscripten::val(2)));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["children"]["0"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["children"]["0"]["textContent"], StrictlyEquals(emscripten::val("foo2")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["children"]["1"]["tagName"], StrictlyEquals(emscripten::val("SPAN")));
        REQUIRE_THAT(node["children"]["1"]["children"]["1"]["children"]["1"]["textContent"], StrictlyEquals(emscripten::val("bar2")));
        REQUIRE_THAT(node["children"]["2"]["tagName"], StrictlyEquals(emscripten::val("DIV")));
        REQUIRE_THAT(node["children"]["2"]["children"]["length"], StrictlyEquals(emscripten::val(0)));
    }
}
