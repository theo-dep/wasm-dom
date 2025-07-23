#include <catch2/catch_test_macros.hpp>

#include "wasm-dom.hpp"

#include "utils.hpp"

#include <algorithm>
#include <codecvt>
#include <locale>
#include <stdlib.h>

using namespace wasmdom;

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
        REQUIRE_FALSE(node == emscripten::val::null());
    } else {
        REQUIRE(node.strictlyEquals(emscripten::val::null()));
    }
    return true;
}

bool refCallbackWithChecks2(emscripten::val node)
{
    return refCallbackWithChecks(node);
}

VNode* spanNum(int i)
{
    return h("span",
             Data(
                 Attrs{
                     { "key", std::to_string(i) } }),
             std::to_string(i));
}

VNode* spanNumWithOpacity(int z, std::string o)
{
    std::string zString = std::to_string(z);
    std::string opacity = std::string("opacity: ");
    opacity.append(o);
    return h("span",
             Data(
                 Attrs{
                     { "key", zString },
                     { "style", opacity } }),
             zString);
}

std::vector<int> shuffle(std::vector<int>& arr, int elms)
{
    std::vector<int> newArr;
    newArr.reserve(elms);
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

    SECTION("should patch a node")
    {
        REQUIRE(emscripten::val::global("document")["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(emscripten::val::global("document")["body"]["firstChild"].strictlyEquals(emscripten::val(getRoot())));
        VNode* vnode = h("span");
        patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(emscripten::val::global("document")["body"]["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["className"].strictlyEquals(emscripten::val("")));
        deleteVNode(vnode);
    }

    SECTION("should patch the same node")
    {
        VNode* vnode = h("div");
        VNode* elmPtr = patch(getRoot(), vnode);
        VNode* elmSamePtr = patch(vnode, elmPtr);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        deleteVNode(elmSamePtr);
    }

    SECTION("should have a tag")
    {
        VNode* vnode = h("div");
        VNode* elmPtr = patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        deleteVNode(elmPtr);
    }

    SECTION("should have the correct namespace")
    {
        std::string svgNamespace = "http://www.w3.org/2000/svg";
        VNode* vnode = h("div",
                         h("div",
                           Data(
                               Attrs{
                                   { "ns", svgNamespace } })));
        patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["firstChild"]["namespaceURI"].strictlyEquals(emscripten::val(svgNamespace)));
        deleteVNode(vnode);
    }

    SECTION("should inject svg namespace")
    {
        std::string svgNamespace = "http://www.w3.org/2000/svg";
        std::string XHTMLNamespace = "http://www.w3.org/1999/xhtml";
        VNode* vnode = h("svg",
                         Children{
                             h("foreignObject",
                               Children{
                                   h("div",
                                     Children{
                                         h("I am HTML embedded in SVG", true) }) }) });

        patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["namespaceURI"].strictlyEquals(emscripten::val(svgNamespace)));
        REQUIRE(elm["firstChild"]["namespaceURI"].strictlyEquals(emscripten::val(svgNamespace)));
        REQUIRE(elm["firstChild"]["firstChild"]["namespaceURI"].strictlyEquals(emscripten::val(XHTMLNamespace)));
        deleteVNode(vnode);
    }

    SECTION("should create elements with class")
    {
        VNode* vnode = h("div",
                         Data(
                             Attrs{
                                 { "class", "foo" } }));
        VNode* elmPtr = patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("class")).strictlyEquals(emscripten::val("foo")));
        deleteVNode(elmPtr);
    }

    SECTION("should create elements with text content")
    {
        VNode* vnode = h("div",
                         Children{
                             h("I am a string", true) });
        VNode* elmPtr = patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["innerHTML"].strictlyEquals(emscripten::val("I am a string")));
        deleteVNode(elmPtr);
    }

    // TODO : how can we test this?
    SECTION("should create elements with text content in utf8") {}

    SECTION("should create elements with span and text content")
    {
        VNode* vnode = h("a",
                         Children{
                             h("span"),
                             h("I am a string", true) });
        VNode* elmPtr = patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("I am a string")));
        deleteVNode(elmPtr);
    }

    SECTION("is a patch of the root element")
    {
        emscripten::val elmWithIdAndClass = emscripten::val::global("document").call<emscripten::val>("createElement", emscripten::val("div"));
        elmWithIdAndClass.set("id", emscripten::val("id"));
        elmWithIdAndClass.set("className", emscripten::val("class"));
        VNode* vnode = h("div",
                         Data(
                             Attrs{
                                 { "id", "id" },
                                 { "class", "class" } }),
                         Children{
                             h(std::string("span"), std::string("Hi")) });

        patch(elmWithIdAndClass, vnode);
        emscripten::val elm = getNode(vnode);
        REQUIRE(elm.strictlyEquals(elmWithIdAndClass));
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["id"].strictlyEquals(emscripten::val("id")));
        REQUIRE(elm["className"].strictlyEquals(emscripten::val("class")));
        deleteVNode(vnode);
    }

    SECTION("should create comments")
    {
        VNode* vnode = h(std::string("!"), std::string("test"));
        VNode* elmPtr = patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeType"].strictlyEquals(emscripten::val::global("document")["COMMENT_NODE"]));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("test")));
        deleteVNode(elmPtr);
    }

    SECTION("should create fragments")
    {
        VNode* vnode = h("", h("foo", true));
        VNode* elmPtr = patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeType"].strictlyEquals(emscripten::val::global("document")["TEXT_NODE"]));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("foo")));
        deleteVNode(elmPtr);
    }

    SECTION("should patch an element inside a fragment")
    {
        VNode* vnode1 = h("", h("span", std::string("foo")));
        VNode* vnode2 = h("", h("span", std::string("bar")));
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("foo")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("bar")));
        deleteVNode(vnode2);
    }

    SECTION("should append elements to fragment")
    {
        VNode* vnode1 = h("div",
                          h("",
                            Children{
                                h("span", std::string("foo")) }));
        VNode* vnode2 = h("div",
                          h("",
                            Children{
                                h("span", std::string("foo")),
                                h("span", std::string("bar")) }));
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
        deleteVNode(vnode2);
    }

    SECTION("should remove elements from fragment")
    {
        VNode* vnode1 = h("div",
                          h("",
                            Children{
                                h("span", std::string("foo")),
                                h("span", std::string("bar")) }));
        VNode* vnode2 = h("div",
                          h("",
                            Children{
                                h("span", std::string("foo")) }));
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["1"]["textContent"].strictlyEquals(emscripten::val("bar")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["textContent"].strictlyEquals(emscripten::val("foo")));
        deleteVNode(vnode2);
    }

    // TODO
    SECTION("should create an element created inside an iframe") {}

    SECTION("should append elements")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        deleteVNode(vnode2);
    }

    SECTION("should prepend elements")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        deleteVNode(vnode2);
    }

    SECTION("should add elements in the middle")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        deleteVNode(vnode2);
    }

    SECTION("should add elements at begin and end")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(2),
                              spanNum(3),
                              spanNum(4) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        deleteVNode(vnode2);
    }

    SECTION("should add children to parent with no children")
    {
        VNode* vnode1 = h("span",
                          Data(
                              Attrs{
                                  { "key", "span" } }));
        VNode* vnode2 = h("span",
                          Data(
                              Attrs{
                                  { "key", "span" } }),
                          Children{
                              h(std::string("span"), std::string("1")),
                              h(std::string("span"), std::string("2")),
                              h(std::string("span"), std::string("3")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        deleteVNode(vnode2);
    }

    SECTION("should remove all children from parent")
    {
        VNode* vnode1 = h("span",
                          Data(
                              Attrs{
                                  { "key", "span" } }),
                          Children{
                              h(std::string("span"), std::string("1")),
                              h(std::string("span"), std::string("2")),
                              h(std::string("span"), std::string("3")) });
        VNode* vnode2 = h("span",
                          Data(
                              Attrs{
                                  { "key", "span" } }));
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        deleteVNode(vnode2);
    }

    SECTION("should update one child with same key but different sel")
    {
        VNode* vnode1 = h("span",
                          Data(
                              Attrs{
                                  { "key", "span" } }),
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3) });
        VNode* vnode2 = h("span",
                          Data(
                              Attrs{
                                  { "key", "span" } }),
                          Children{
                              spanNum(1),
                              h("i",
                                Data(
                                    Attrs{
                                        { "key", "2" } }),
                                std::string("2")),
                              spanNum(3) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("I")));
        deleteVNode(vnode2);
    }

    SECTION("should remove elements from the beginning")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        deleteVNode(vnode2);
    }

    SECTION("should remove elements from the end")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        deleteVNode(vnode2);
    }

    SECTION("should remove elements from the middle")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(4),
                              spanNum(5) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        deleteVNode(vnode2);
    }

    SECTION("should move element forward")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(2),
                              spanNum(3),
                              spanNum(1),
                              spanNum(4) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        deleteVNode(vnode2);
    }

    SECTION("should move element to end")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(2),
                              spanNum(3),
                              spanNum(1) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        deleteVNode(vnode2);
    }

    SECTION("should move element backwards")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(4),
                              spanNum(2),
                              spanNum(3) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        deleteVNode(vnode2);
    }

    SECTION("should swap first and last")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(4),
                              spanNum(2),
                              spanNum(3),
                              spanNum(1) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        deleteVNode(vnode2);
    }

    SECTION("should move to left and replace")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(4),
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(6) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(5)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("6")));
        deleteVNode(vnode2);
    }

    SECTION("should move to left and leaves hole")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(4),
                              spanNum(6) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("6")));
        deleteVNode(vnode2);
    }

    SECTION("should handle moved and set to undefined element ending at the end")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(2),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(4),
                              spanNum(5),
                              spanNum(3) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        deleteVNode(vnode2);
    }

    SECTION("should move a key in non-keyed nodes with a size up")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              h(std::string("span"), std::string("a")),
                              h(std::string("span"), std::string("b")),
                              h(std::string("span"), std::string("c")) });
        VNode* vnode2 = h("span",
                          Children{
                              h(std::string("span"), std::string("d")),
                              h(std::string("span"), std::string("a")),
                              h(std::string("span"), std::string("b")),
                              h(std::string("span"), std::string("c")),
                              spanNum(1),
                              h(std::string("span"), std::string("e")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(4)));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("1abc")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("dabc1e")));
        deleteVNode(vnode2);
    }

    SECTION("should reverse elements")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5),
                              spanNum(6),
                              spanNum(7),
                              spanNum(8) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(8),
                              spanNum(7),
                              spanNum(6),
                              spanNum(5),
                              spanNum(4),
                              spanNum(3),
                              spanNum(2),
                              spanNum(1) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(8)));
        patch(vnode1, vnode2);
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
        deleteVNode(vnode2);
    }

    SECTION("should reverse elements with 0")
    {
        VNode* vnode1 = h("span",
                          Children{
                              spanNum(0),
                              spanNum(1),
                              spanNum(2),
                              spanNum(3),
                              spanNum(4),
                              spanNum(5) });
        VNode* vnode2 = h("span",
                          Children{
                              spanNum(4),
                              spanNum(3),
                              spanNum(2),
                              spanNum(1),
                              spanNum(5),
                              spanNum(0) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("0")));
        deleteVNode(vnode2);
    }

    SECTION("should handle random shuffles")
    {
        int n;
        int i;
        std::vector<int> arr;
        std::vector<std::string> opacities;
        int elms = 14;
        int samples = 5;

        arr.reserve(elms);
        opacities.reserve(elms);
        for (n = 0; n < elms; ++n) {
            arr[n] = n;
        }
        for (n = 0; n < samples; ++n) {
            Children children;
            for (i = 0; i < elms; ++i) {
                children.push_back(spanNumWithOpacity(arr[i], std::string("1")));
            }
            VNode* vnode1 = h("span", children);

            std::vector<int> shufArr = shuffle(arr, elms);

            emscripten::val elm = emscripten::val::global("document").call<emscripten::val>("createElement", emscripten::val("div"));
            patch(elm, vnode1);
            elm = getNode(vnode1);
            for (i = 0; i < elms; ++i) {
                REQUIRE(elm["children"][std::to_string(i)]["innerHTML"].strictlyEquals(emscripten::val(std::to_string(i))));
                opacities[i] = std::string("0.");
                opacities[i].append(std::to_string(rand() % 99999));
            }
            Children opacityChildren;
            for (i = 0; i < elms; ++i) {
                opacityChildren.push_back(spanNumWithOpacity(shufArr[i], opacities[i]));
            }
            VNode* vnode2 = h("span", opacityChildren);

            patch(vnode1, vnode2);
            elm = getNode(vnode2);
            for (i = 0; i < elms; ++i) {
                REQUIRE(elm["children"][std::to_string(i)]["innerHTML"].strictlyEquals(emscripten::val(std::to_string(shufArr[i]))));
                REQUIRE(emscripten::val(opacities[i]).call<emscripten::val>("indexOf", elm["children"][std::to_string(i)]["style"]["opacity"]).strictlyEquals(emscripten::val(0)));
            }
            deleteVNode(vnode2);
        }
    }

    SECTION("should support null/undefined children")
    {
        VNode* vnode1 = h("span",
                          Children{
                              h(std::string("span"), std::string("0")),
                              h(std::string("span"), std::string("1")),
                              h(std::string("span"), std::string("2")),
                              h(std::string("span"), std::string("3")),
                              h(std::string("span"), std::string("4")),
                              h(std::string("span"), std::string("5")) });
        VNode* vnode2 = h("span",
                          Children{
                              nullptr,
                              h(std::string("span"), std::string("2")),
                              nullptr,
                              nullptr,
                              h(std::string("span"), std::string("1")),
                              h(std::string("span"), std::string("0")),
                              nullptr,
                              h(std::string("span"), std::string("5")),
                              h(std::string("span"), std::string("4")),
                              nullptr,
                              h(std::string("span"), std::string("3")),
                              nullptr });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("0")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        deleteVNode(vnode2);
    }

    SECTION("should support all null/undefined children")
    {
        VNode* vnode1 = h("span",
                          Children{
                              h(std::string("span"), std::string("0")),
                              h(std::string("span"), std::string("1")),
                              h(std::string("span"), std::string("2")),
                              h(std::string("span"), std::string("3")),
                              h(std::string("span"), std::string("4")),
                              h(std::string("span"), std::string("5")) });
        VNode* vnode2 = h("span",
                          Children{
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr,
                              nullptr });
        VNode* vnode3 = h("span",
                          Children{
                              h(std::string("span"), std::string("5")),
                              h(std::string("span"), std::string("4")),
                              h(std::string("span"), std::string("3")),
                              h(std::string("span"), std::string("2")),
                              h(std::string("span"), std::string("1")),
                              h(std::string("span"), std::string("0")) });
        patch(getRoot(), vnode1);
        patch(vnode1, vnode2);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        patch(vnode2, vnode3);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(6)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("5")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("4")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("3")));
        REQUIRE(elm["children"]["3"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["4"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["5"]["innerHTML"].strictlyEquals(emscripten::val("0")));
        deleteVNode(vnode3);
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
        VNode* vnode1 = patch(getRoot(), h("div"));
        VNode* vnode2;
        for (i = 0; i < samples; ++i) {
            len = rand() % (maxArrLen + 1);
            arr = std::vector<int>();
            arr.reserve(len);
            for (j = 0; j < len; ++j) {
                r = rand() % 100;
                if (r < 50)
                    arr[j] = j;
                else
                    arr[j] = 0;
            }
            std::vector<int> shufArr = shuffle(arr, len);
            Children children = Children();
            for (j = 0; j < len; ++j) {
                children.push_back(shufArr[j] == 0 ? 0 : h(std::string("span"), std::to_string(shufArr[j])));
            }
            vnode2 = h("div", children);
            patch(vnode1, vnode2);
            emscripten::val elm = getBodyFirstChild();
            r = 0;
            for (j = 0; j < len; ++j) {
                if (shufArr[j] != 0) {
                    REQUIRE(elm["children"][std::to_string(r)]["innerHTML"].strictlyEquals(emscripten::val(std::to_string(shufArr[j]))));
                    ++r;
                }
            }
            REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(r)));
            vnode1 = vnode2;
        }
        deleteVNode(vnode2);
    }

    SECTION("should append elements")
    {
        VNode* vnode1 = h("span",
                          Children{
                              h(std::string("span"), std::string("Hello")) });
        VNode* vnode2 = h("span",
                          Children{
                              h(std::string("span"), std::string("Hello")),
                              h(std::string("span"), std::string("World")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        deleteVNode(vnode2);
    }

    SECTION("should handle unmoved text nodes")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h("Text", true),
                              h(std::string("span"), std::string("Span")) });
        VNode* vnode2 = h("div",
                          Children{
                              h("Text", true),
                              h(std::string("span"), std::string("Span")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        deleteVNode(vnode2);
    }

    SECTION("should handle changing text children")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h("Text", true),
                              h(std::string("span"), std::string("Span")) });
        VNode* vnode2 = h("div",
                          Children{
                              h("Text2", true),
                              h(std::string("span"), std::string("Span")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text2")));
        deleteVNode(vnode2);
    }

    SECTION("should handle unmoved comment nodes")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h(std::string("!"), std::string("Text")),
                              h(std::string("span"), std::string("Span")) });
        VNode* vnode2 = h("div",
                          Children{
                              h(std::string("!"), std::string("Text")),
                              h(std::string("span"), std::string("Span")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        deleteVNode(vnode2);
    }

    SECTION("should handle changing comment text")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h(std::string("!"), std::string("Text")),
                              h(std::string("span"), std::string("Span")) });
        VNode* vnode2 = h("div",
                          Children{
                              h(std::string("!"), std::string("Text2")),
                              h(std::string("span"), std::string("Span")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Text2")));
        deleteVNode(vnode2);
    }

    SECTION("should handle changing empty comment")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h("!"),
                              h(std::string("span"), std::string("Span")) });
        VNode* vnode2 = h("div",
                          Children{
                              h(std::string("!"), std::string("Test")),
                              h(std::string("span"), std::string("Span")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Test")));
        deleteVNode(vnode2);
    }

    SECTION("should prepend element")
    {
        VNode* vnode1 = h("span",
                          Children{
                              h(std::string("span"), std::string("World")) });
        VNode* vnode2 = h("span",
                          Children{
                              h(std::string("span"), std::string("Hello")),
                              h(std::string("span"), std::string("World")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        deleteVNode(vnode2);
    }

    SECTION("should prepend element of different tag type")
    {
        VNode* vnode1 = h("span",
                          Children{
                              h(std::string("span"), std::string("World")) });
        VNode* vnode2 = h("span",
                          Children{
                              h(std::string("div"), std::string("Hello")),
                              h(std::string("span"), std::string("World")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Hello")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("World")));
        deleteVNode(vnode2);
    }

    SECTION("should remove elements")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h(std::string("span"), std::string("One")),
                              h(std::string("span"), std::string("Two")),
                              h(std::string("span"), std::string("Three")) });
        VNode* vnode2 = h("div",
                          Children{
                              h(std::string("span"), std::string("One")),
                              h(std::string("span"), std::string("Three")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        deleteVNode(vnode2);
    }

    SECTION("should remove a single text node")
    {
        VNode* vnode1 = h(std::string("div"), std::string("One"));
        VNode* vnode2 = h("div");
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("One")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("")));
        deleteVNode(vnode2);
    }

    SECTION("should remove a single text node when children are updated")
    {
        VNode* vnode1 = h(std::string("div"), std::string("One"));
        VNode* vnode2 = h("div",
                          Children{
                              h(std::string("div"), std::string("Two")),
                              h(std::string("span"), std::string("Three")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["textContent"].strictlyEquals(emscripten::val("One")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("Three")));
        deleteVNode(vnode2);
    }

    SECTION("should remove a text node among other elements")
    {
        VNode* vnode1 = h("div",
                          Children{
                              h(std::string("One"), true),
                              h(std::string("span"), std::string("Two")) });
        VNode* vnode2 = h("div",
                          Children{
                              h(std::string("div"), std::string("Three")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["childNodes"]["1"]["textContent"].strictlyEquals(emscripten::val("Two")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["childNodes"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["childNodes"]["0"]["tagName"].strictlyEquals(emscripten::val("DIV")));
        REQUIRE(elm["childNodes"]["0"]["textContent"].strictlyEquals(emscripten::val("Three")));
        deleteVNode(vnode2);
    }

    SECTION("should reorder elements")
    {
        VNode* vnode1 = h("span",
                          Children{
                              h(std::string("span"), std::string("One")),
                              h(std::string("div"), std::string("Two")),
                              h(std::string("b"), std::string("Three")) });
        VNode* vnode2 = h("span",
                          Children{
                              h(std::string("b"), std::string("Three")),
                              h(std::string("span"), std::string("One")),
                              h(std::string("div"), std::string("Two")) });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(3)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("Three")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("One")));
        REQUIRE(elm["children"]["2"]["innerHTML"].strictlyEquals(emscripten::val("Two")));
        REQUIRE(elm["children"]["0"]["tagName"].strictlyEquals(emscripten::val("B")));
        REQUIRE(elm["children"]["1"]["tagName"].strictlyEquals(emscripten::val("SPAN")));
        REQUIRE(elm["children"]["2"]["tagName"].strictlyEquals(emscripten::val("DIV")));
        deleteVNode(vnode2);
    }

    SECTION("should support null/undefined children")
    {
        VNode* vnode1 = h("i",
                          Children{
                              nullptr,
                              h(std::string("i"), std::string("1")),
                              h(std::string("i"), std::string("2")),
                              nullptr });
        VNode* vnode2 = h("i",
                          Children{
                              h(std::string("i"), std::string("2")),
                              nullptr,
                              nullptr,
                              h(std::string("i"), std::string("1")),
                              nullptr });
        VNode* vnode3 = h("i",
                          Children{
                              nullptr,
                              h(std::string("i"), std::string("1")),
                              nullptr,
                              nullptr,
                              h(std::string("i"), std::string("2")),
                              nullptr,
                              nullptr });
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        patch(vnode1, vnode2);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        patch(vnode2, vnode3);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(2)));
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        deleteVNode(vnode3);
    }

    SECTION("should support all null/undefined children")
    {
        VNode* vnode1 = h("i",
                          Children{
                              h(std::string("i"), std::string("1")),
                              h(std::string("i"), std::string("2")) });
        VNode* vnode2 = h("i",
                          Children{
                              nullptr,
                              nullptr });
        VNode* vnode3 = h("i",
                          Children{
                              h(std::string("i"), std::string("2")),
                              h(std::string("i"), std::string("1")) });
        patch(getRoot(), vnode1);
        patch(vnode1, vnode2);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["children"]["length"].strictlyEquals(emscripten::val(0)));
        patch(vnode2, vnode3);
        elm = getBodyFirstChild();
        REQUIRE(elm["children"]["0"]["innerHTML"].strictlyEquals(emscripten::val("2")));
        REQUIRE(elm["children"]["1"]["innerHTML"].strictlyEquals(emscripten::val("1")));
        deleteVNode(vnode3);
    }

    SECTION("should set asmDomRaws")
    {
        VNode* vnode1 = h("i",
                          Data(
                              Props{
                                  { "foo", emscripten::val("") } }));
        VNode* vnode2 = h("i",
                          Data(
                              Props{
                                  { "bar", emscripten::val("") } }));
        VNode* vnode3 = h("i");
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["asmDomRaws"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["asmDomRaws"]["0"].strictlyEquals(emscripten::val("foo")));
        patch(vnode1, vnode2);
        REQUIRE(elm["asmDomRaws"]["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(elm["asmDomRaws"]["0"].strictlyEquals(emscripten::val("bar")));
        patch(vnode2, vnode3);
        REQUIRE(elm["asmDomRaws"]["length"].strictlyEquals(emscripten::val(0)));
        deleteVNode(vnode3);
    }

    SECTION("should set asmDomEvents")
    {
        VNode* vnode1 = h("i",
                          Data(
                              Callbacks{
                                  { "onclick", onClick } }));
        VNode* vnode2 = h("i",
                          Data(
                              Callbacks{
                                  { "onkeydown", onClick } }));
        VNode* vnode3 = h("i");
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(keys["0"].strictlyEquals(emscripten::val("click")));
        patch(vnode1, vnode2);
        keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(keys["0"].strictlyEquals(emscripten::val("keydown")));
        patch(vnode2, vnode3);
        keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(0)));
        deleteVNode(vnode3);
    }

    SECTION("should patch a WebComponent")
    {
        VNode* vnode = h("web-component");
        patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("WEB-COMPONENT")));
        deleteVNode(vnode);
    }

    SECTION("should patch a WebComponent with attributes")
    {
        VNode* vnode = h("web-component",
                         Data(
                             Attrs{
                                 { "foo", "bar" },
                                 { "bar", "42" } }));
        patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("WEB-COMPONENT")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("foo")).strictlyEquals(emscripten::val("bar")));
        REQUIRE(elm.call<emscripten::val>("getAttribute", emscripten::val("bar")).strictlyEquals(emscripten::val("42")));
        deleteVNode(vnode);
    }

    SECTION("should patch a WebComponent with eventListeners")
    {
        VNode* vnode = h("web-component",
                         Data(
                             Callbacks{
                                 { "onclick", onClick },
                                 { "onfoo-event", onClick } }));
        patch(getRoot(), vnode);
        emscripten::val elm = getBodyFirstChild();
        REQUIRE(elm["nodeName"].strictlyEquals(emscripten::val("WEB-COMPONENT")));
        deleteVNode(vnode);
    }

    SECTION("should create a template node")
    {
        VNode* vnode = h("template",
                         Data(
                             Attrs{
                                 { "id", "template-node" } }),
                         Children{
                             h("style", std::string("p { color: green; }")),
                             h("p", std::string("Hello world!")) });
        patch(getRoot(), vnode);
        emscripten::val tmpl = emscripten::val::global("document").call<emscripten::val>("getElementById", emscripten::val("template-node"));
        emscripten::val fragment = tmpl["content"].call<emscripten::val>("cloneNode", emscripten::val(true));
        REQUIRE(fragment["nodeName"].strictlyEquals(emscripten::val("#document-fragment")));
        deleteVNode(vnode);
    }

    SECTION("should call ref with DOM node")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Attrs{
                                    { "data-foo", "bar" } },
                                Callbacks{
                                    { "ref", [&](emscripten::val node) -> bool {
                                         ++refCount;
                                         if (refCount == 2) {
                                             REQUIRE(node.call<emscripten::val>("getAttribute", emscripten::val("data-foo")).strictlyEquals(emscripten::val("bar")));
                                         } else {
                                             REQUIRE(node.strictlyEquals(emscripten::val::null()));
                                         }
                                         return true;
                                     } } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div");
        patch(vnode1, vnode2);

        REQUIRE(refCount == 3);

        deleteVNode(vnode2);
    }

    SECTION("should call ref on add")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallback } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        deleteVNode(vnode1);
    }

    SECTION("should call ref on remove")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallback } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div");
        patch(vnode1, vnode2);

        REQUIRE(refCount == 3);

        deleteVNode(vnode2);
    }

    SECTION("should call ref on ref remove itself")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallback } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div", h("div"));
        patch(vnode1, vnode2);

        REQUIRE(refCount == 3);

        deleteVNode(vnode2);
    }

    SECTION("should not call ref on update")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallback } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallback } })));
        patch(vnode1, vnode2);

        REQUIRE(refCount == 2);

        deleteVNode(vnode2);
    }

    SECTION("should call ref on change (lambda - lambda)")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", [&](emscripten::val e) -> bool {
                                         refCallbackWithChecks(e);
                                         return true;
                                     } } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", [&](emscripten::val e) -> bool {
                                         refCallbackWithChecks(e);
                                         return false;
                                     } } })));
        patch(vnode1, vnode2);

        REQUIRE(refCount == 4);

        deleteVNode(vnode2);
    }

    SECTION("should call ref on change (pointer - lambda)")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", [&](emscripten::val e) -> bool {
                                         refCallbackWithChecks(e);
                                         return false;
                                     } } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallbackWithChecks } })));
        patch(vnode1, vnode2);

        REQUIRE(refCount == 4);

        deleteVNode(vnode2);
    }

    SECTION("should call ref on change (pointer - pointer)")
    {
        refCount = 1;

        VNode* vnode1 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallbackWithChecks } })));
        patch(getRoot(), vnode1);

        REQUIRE(refCount == 2);

        VNode* vnode2 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallbackWithChecks2 } })));
        patch(vnode1, vnode2);

        REQUIRE(refCount == 4);

        deleteVNode(vnode2);
    }

    SECTION("should call ref on update if ref is added")
    {
        refCount = 1;

        VNode* vnode1 = h("div", h("div"));
        patch(getRoot(), vnode1);

        VNode* vnode2 = h("div",
                          h("div",
                            Data(
                                Callbacks{
                                    { "ref", refCallback } })));
        patch(vnode1, vnode2);

        REQUIRE(refCount == 2);

        deleteVNode(vnode2);
    }

    SECTION("should not set ref as callback")
    {
        VNode* vnode1 = h("i",
                          Data(
                              Callbacks{
                                  { "onclick", onClick },
                                  { "ref", onClick } }));
        patch(getRoot(), vnode1);
        emscripten::val elm = getBodyFirstChild();
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", elm["asmDomEvents"]);
        REQUIRE(keys["length"].strictlyEquals(emscripten::val(1)));
        REQUIRE(keys["0"].strictlyEquals(emscripten::val("click")));
        deleteVNode(vnode1);
    }
}
