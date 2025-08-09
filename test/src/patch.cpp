#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

int refCount = 1;
bool refCallback(emscripten::val /*node*/)
{
    ++refCount;
    return true;
}

bool refCallback2(emscripten::val node)
{
    return refCallback(node);
}

bool refCallbackWithChecks(emscripten::val node)
{
    ++refCount;
    if (refCount % 2 == 0) {
        REQUIRE_FALSE(node.isNull());
    } else {
        REQUIRE(node.isNull());
    }
    return true;
}

bool refCallbackWithChecks2(emscripten::val node)
{
    return refCallbackWithChecks(node);
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

std::vector<int> shuffle(std::vector<int>& arr, int elms)
{
    std::vector<int> newArr;
    newArr.resize(elms);
    for (int n = 0; n < elms; ++n) {
        newArr[n] = arr[n];
    }
    for (int n = 0; n < elms; ++n) {
        int i = rand() % elms;

        int temp = newArr[n];
        newArr[n] = newArr[i];
        newArr[i] = temp;
    }
    return newArr;
}

TEST_CASE("patch", "[patch]")
{
    setupDom();

    SECTION("should handle nullptr VNode")
    {
        REQUIRE(emscripten::val::global("document")["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(emscripten::val::global("document")["body"]["firstChild"].strictlyEquals(emscripten::val(getRoot())));
        VNode vnode = nullptr;
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(emscripten::val::global("document")["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(emscripten::val::global("document")["body"]["firstChild"].strictlyEquals(emscripten::val(getRoot())));
    }

    SECTION("should patch a node")
    {
        REQUIRE(emscripten::val::global("document")["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(emscripten::val::global("document")["body"]["firstChild"].strictlyEquals(emscripten::val(getRoot())));
        VNode vnode = span();
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(emscripten::val::global("document")["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["className"].strictlyEquals(emscripten::val("")));
    }

    SECTION("should patch the same node")
    {
        VNode vnode = div();
        VDom vdom(getRoot());
        VNode elmPtr = vdom.patch(vnode);
        vdom.patch(elmPtr);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should have a tag")
    {
        VNode vnode = div();
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
    }

    SECTION("should have the correct namespace")
    {
        std::string svgNamespace = "http://www.w3.org/2000/svg";
        VNode vnode =
            div()(
                div(("ns", svgNamespace))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["firstChild"]["namespaceURI"].strictlyEquals(emscripten::val(svgNamespace)));
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

        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["namespaceURI"].strictlyEquals(emscripten::val(svgNamespace)));
        REQUIRE(elm["firstChild"]["namespaceURI"].strictlyEquals(emscripten::val(svgNamespace)));
        REQUIRE(elm["firstChild"]["firstChild"]["namespaceURI"].strictlyEquals(emscripten::val(XHTMLNamespace)));
    }

    SECTION("should create elements with class")
    {
        VNode vnode = div(("class", "foo"s));
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("class")).strictlyEquals(emscripten::val("foo")));
    }

    SECTION("should create elements with text content")
    {
        VNode vnode =
            div()(
                t("I am a string")
            );
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["innerHTML"].strictlyEquals(emscripten::val("I am a string")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("I am a string")));
    }

    SECTION("is a patch of the root element")
    {
        emscripten::val elmWithIdAndClass = emscripten::val::global("document").call<emscripten::val>("createElement", emscripten::val("div"));
        elmWithIdAndClass.set("id", emscripten::val("id"));
        elmWithIdAndClass.set("className", emscripten::val("class"));
        VNode vnode =
            div(("id", "id"s),
                ("class", "class"s))(
                span()(std::string("Hi"))
            );

        VDom vdom(elmWithIdAndClass);
        vdom.patch(vnode);
        emscripten::val elm = domapi::node(vnode.elm());
        REQUIRE(elm.strictlyEquals(elmWithIdAndClass));
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["id"].strictlyEquals(emscripten::val("id")));
        REQUIRE(elm["className"].strictlyEquals(emscripten::val("class")));
    }

    SECTION("should create comments")
    {
        VNode vnode = comment(std::string("test"));
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeType"].strictlyEquals(emscripten::val::global("document")["COMMENT_NODE"]));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("test")));
    }

    SECTION("should create fragments")
    {
        VNode vnode =
            fragment()(
                t("foo")
            );
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeType"].strictlyEquals(emscripten::val::global("document")["TEXT_NODE"]));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("foo")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("bar")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("I")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("5")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("5")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("1")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("3")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("1")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("6")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("6")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("1abc")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("dabc1e")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(8)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(8)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("8")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("7")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("6")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["6"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["7"]["innerHTML"].strictlyEquals(emscripten::val("1")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("0")));
    }

    SECTION("should handle random shuffles")
    {
        int n;
        int i;
        std::vector<int> arr;
        std::vector<std::string> opacities;
        int elms = 14;
        int samples = 5;

        arr.resize(elms);
        opacities.resize(elms);
        for (n = 0; n < elms; ++n) {
            arr[n] = n;
        }

        for (n = 0; n < samples; ++n) {
            Children children;
            for (i = 0; i < elms; ++i) {
                children.push_back(spanNumWithOpacity(arr[i], std::string("1")));
            }
            VNode vnode1 = span()(children);

            std::vector<int> shufArr = shuffle(arr, elms);

            emscripten::val elm = emscripten::val::global("document").call<emscripten::val>("createElement", emscripten::val("div"));
            VDom vdom(elm);
            vdom.patch(vnode1);
            elm = domapi::node(vnode1.elm());
            for (i = 0; i < elms; ++i) {
                REQUIRE(elm["children"][std::to_string(i)]["innerHTML"].strictlyEquals(emscripten::val(std::to_string(i))));
                opacities[i] = std::string("0.");
                opacities[i].append(std::to_string(rand() % 99999));
            }
            Children opacityChildren;
            for (i = 0; i < elms; ++i) {
                opacityChildren.push_back(spanNumWithOpacity(shufArr[i], opacities[i]));
            }
            VNode vnode2 = span()(opacityChildren);

            vdom.patch(vnode2);
            elm = domapi::node(vnode2.elm());
            for (i = 0; i < elms; ++i) {
                REQUIRE(elm["children"][std::to_string(i)]["innerHTML"].strictlyEquals(emscripten::val(std::to_string(shufArr[i]))));
                REQUIRE(emscripten::val(opacities[i]).call<emscripten::val>("indexOf", elm["children"][std::to_string(i)]["style"]["opacity"]).strictlyEquals(emscripten::val(0)));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("0")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("3")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        vdom.patch(vnode2);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("0")));
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

            emscripten::val elm = emscripten::val::global("document").call<emscripten::val>("createElement", emscripten::val("div"));
            VDom vdom(elm);
            vdom.patch(div());

            Children children = Children();
            for (j = 0; j < len; ++j) {
                children.push_back(shufArr[j] == 0 ? nullptr : span()(std::to_string(shufArr[j])));
            }
            VNode vnode = div()(children);
            vdom.patch(vnode);
            elm = domapi::node(vnode.elm());
            r = 0;
            for (j = 0; j < len; ++j) {
                if (shufArr[j] != 0) {
                    REQUIRE(elm["children"][std::to_string(r)]["innerHTML"].strictlyEquals(emscripten::val(std::to_string(shufArr[j]))));
                    ++r;
                }
            }
            REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(r)));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("World")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text2")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text2")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Test")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("World")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("World")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
    }

    SECTION("should remove a single text node")
    {
        VNode vnode1 = div()(std::string("One"));
        VNode vnode2 = div();
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("One")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("")));
    }

    SECTION("should remove a single text node when children are updated")
    {
        VNode vnode1 = div()(std::string("One"));
        VNode vnode2 =
            div()(
                { div()(std::string("Two")),
                  span()(std::string("Three")) }
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("One")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("Three")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("Two")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Three")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("B")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["2"]["tagName"].strictlyEquals(emscripten::val("DIV")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        vdom.patch(vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        vdom.patch(vnode3);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
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
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        vdom.patch(vnode2);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        vdom.patch(vnode3);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
    }

    SECTION("should set asmDomRaws")
    {
        VNode vnode1 = i(("foo", emscripten::val("")));
        VNode vnode2 = i(("bar", emscripten::val("")));
        VNode vnode3 = i();
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["asmDomRaws"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["asmDomRaws"]["0"].strictlyEquals(emscripten::val("foo")));
        vdom.patch(vnode2);
        REQUIRE(elm["asmDomRaws"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["asmDomRaws"]["0"].strictlyEquals(emscripten::val("bar")));
        vdom.patch(vnode3);
        REQUIRE(elm["asmDomRaws"]["length"].strictlyEquals(emscripten::val(0)));
    }

    SECTION("should set asmDomEvents")
    {
        VNode vnode1 = i(("onclick", f(onClick)));
        VNode vnode2 = i(("onkeydown", f(onClick)));
        VNode vnode3 = i();
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(keys["0"].strictlyEquals(emscripten::val("click")));
        vdom.patch(vnode2);
        keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(keys["0"].strictlyEquals(emscripten::val("keydown")));
        vdom.patch(vnode3);
        keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(0)));
    }

    SECTION("should patch a WebComponent")
    {
        VNode vnode = webComponent();
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("WEB-COMPONENT")));
    }

    SECTION("should patch a WebComponent with attributes")
    {
        VNode vnode = webComponent(("foo", "bar"s), ("bar", "42"s));
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("WEB-COMPONENT")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("foo")).strictlyEquals(emscripten::val("bar")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("bar")).strictlyEquals(emscripten::val("42")));
    }

    SECTION("should patch a WebComponent with eventListeners")
    {
        VNode vnode =
            webComponent(("onclick", f(onClick)), ("onfoo-event", f(onClick)));
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("WEB-COMPONENT")));
    }

    SECTION("should create a template node")
    {
        VNode vnode =
            hTemplate(("id", "template-node"s))(
                { style()(" p{ color : green;"),
                  p()(std::string("Hello world!")) }
            );
        VDom vdom(getRoot());
        vdom.patch(vnode);
        emscripten::val tmpl = emscripten::val::global("document").call<emscripten::val>("getElementById", emscripten::val("template-node"));
        emscripten::val fragment = tmpl["content"].call<emscripten::val>("cloneNode", emscripten::val(true));
        REQUIRE(fragment["nodeName"].strictlyEquals(emscripten::val("#document-fragment")));
    }

    SECTION("should call ref with DOM node")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("data-foo", "bar"s),
                    ("ref", [&](emscripten::val node) -> bool {
                        ++refCount;
                        if (refCount == 2) {
                            REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("bar")));
                        } else {
                            REQUIRE(node.isNull());
                        }
                        return true;
                    }))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 = div();
        vdom.patch(vnode2);

        REQUIRE(refCount == 3);
    }

    SECTION("should call ref on add")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", f(refCallback)))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);
    }

    SECTION("should call ref on remove")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", f(refCallback)))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 = div();
        vdom.patch(vnode2);

        REQUIRE(refCount == 3);
    }

    SECTION("should call ref on ref remove itself")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", f(refCallback)))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 =
            div()(
                div()
            );
        vdom.patch(vnode2);

        REQUIRE(refCount == 3);
    }

    SECTION("should not call ref on update")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", f(refCallback)))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 =
            div()(
                div(("ref", f(refCallback)))
            );
        vdom.patch(vnode2);

        REQUIRE(refCount == 2);
    }

    SECTION("should call ref on change (lambda - lambda)")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", [&](emscripten::val e) -> bool {
                    refCallbackWithChecks(e);
                    return true;
                }))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 =
            div()(
                div(("ref", [&](emscripten::val e) -> bool {
                    refCallbackWithChecks(e);
                    return false;
                }))
            );
        vdom.patch(vnode2);

        REQUIRE(refCount == 4);
    }

    SECTION("should call ref on change (pointer - lambda)")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", [&](emscripten::val e) -> bool {
                    refCallbackWithChecks(e);
                    return false;
                }))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 =
            div()(
                div(("ref", f(refCallbackWithChecks)))
            );
        vdom.patch(vnode2);

        REQUIRE(refCount == 4);
    }

    SECTION("should call ref on change (pointer - pointer)")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div(("ref", f(refCallbackWithChecks)))
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        REQUIRE(refCount == 2);

        VNode vnode2 =
            div()(
                div(("ref", f(refCallbackWithChecks2)))
            );
        vdom.patch(vnode2);

        REQUIRE(refCount == 4);
    }

    SECTION("should call ref on update if ref is added")
    {
        refCount = 1;

        VNode vnode1 =
            div()(
                div()
            );
        VDom vdom(getRoot());
        vdom.patch(vnode1);

        VNode vnode2 =
            div()(
                div(("ref", f(refCallback)))
            );
        vdom.patch(vnode2);

        REQUIRE(refCount == 2);
    }

    SECTION("should not set ref as callback")
    {
        VNode vnode1 = i(("onclick", f(onClick)), ("ref", f(onClick)));
        VDom vdom(getRoot());
        vdom.patch(vnode1);
        emscripten::val elm = getBodyFirstChild();
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(keys["0"].strictlyEquals(emscripten::val("click")));
    }
}
