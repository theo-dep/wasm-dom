#include "domapi.hpp"

#include "vnode.hpp"

#include <emscripten.h>

#include <unordered_map>

namespace wasmdom::domapi
{
    std::unordered_map<int, emscripten::val>& nodes()
    {
        static std::unordered_map<int, emscripten::val> nodes{ { 0, emscripten::val::null() } };
        return nodes;
    }

    int addPtr(const emscripten::val& node)
    {
        static int lastPtr = 0;

        if (node == emscripten::val::null() || node == emscripten::val::undefined())
            return 0;
        if (node["asmDomPtr"] != emscripten::val::undefined())
            return node["asmDomPtr"].as<int>();

        emscripten::val newNode = node;

        ++lastPtr;
        newNode.set("asmDomPtr", lastPtr);
        nodes().emplace(lastPtr, newNode);
        return lastPtr;
    }
}

emscripten::val wasmdom::domapi::node(int nodePtr)
{
    if (nodes().contains(nodePtr))
        return nodes()[nodePtr];
    return emscripten::val::null();
}

int wasmdom::domapi::addNode(const emscripten::val& node)
{
    addPtr(node["parentNode"]);
    addPtr(node["nextSibling"]);
    return addPtr(node);
}

int wasmdom::domapi::createElement(const std::string& tag)
{
    return addPtr(emscripten::val::module_property("recycler").call<emscripten::val>("create", tag));
}

int wasmdom::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return addPtr(emscripten::val::module_property("recycler").call<emscripten::val>("createNS", qualifiedName, namespaceURI));
}

int wasmdom::domapi::createTextNode(const std::string& text)
{
    return addPtr(emscripten::val::module_property("recycler").call<emscripten::val>("createText", text));
}

int wasmdom::domapi::createComment(const std::string& text)
{
    return addPtr(emscripten::val::module_property("recycler").call<emscripten::val>("createComment", text));
}

int wasmdom::domapi::createDocumentFragment()
{
    return addPtr(emscripten::val::global("document").call<emscripten::val>("createDocumentFragment"));
}

void wasmdom::domapi::insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr)
{
    if (parentNodePtr == 0 /*|| newNodePtr == 0 || referenceNodePtr == 0*/)
        return;

    node(parentNodePtr).call<void>("insertBefore", node(newNodePtr), node(referenceNodePtr));
}

void wasmdom::domapi::removeChild(int childPtr)
{
    emscripten::val node = domapi::node(childPtr);
    if (node == emscripten::val::null() || node == emscripten::val::undefined())
        return;

    emscripten::val parentNode = node["parentNode"];
    if (parentNode != emscripten::val::null())
        parentNode.call<void>("removeChild", node);

    emscripten::val::module_property("recycler").call<void>("collect", node);
}

void wasmdom::domapi::appendChild(int parentPtr, int childPtr)
{
    node(parentPtr).call<void>("appendChild", node(childPtr));
}

void wasmdom::domapi::removeAttribute(int nodePtr, const std::string& attribute)
{
    node(nodePtr).call<void>("removeAttribute", attribute);
}

void wasmdom::domapi::setAttribute(int nodePtr, const std::string& attribute, const std::string& value)
{
    emscripten::val node = domapi::node(nodePtr);
    if (attribute[0] != 'x') {
        node.call<void>("setAttribute", attribute, value);
    } else if (attribute[3] == ':') {
        // Assume xml namespace
        node.call<void>("setAttributeNS", std::string("http://www.w3.org/XML/1998/namespace"), attribute, value);
    } else if (attribute[5] == ':') {
        // Assume xlink namespace
        node.call<void>("setAttributeNS", std::string("http://www.w3.org/1999/xlink"), attribute, value);
    } else {
        node.call<void>("setAttribute", attribute, value);
    }
}

void wasmdom::domapi::setNodeValue(int nodePtr, const std::string& text)
{
    node(nodePtr).set("nodeValue", text);
}

int wasmdom::domapi::parentNode(int nodePtr)
{
    emscripten::val node = domapi::node(nodePtr);
    if (node != emscripten::val::null() && node != emscripten::val::undefined() && node["parentNode"] != emscripten::val::null())
        return node["parentNode"]["asmDomPtr"].as<int>();
    return 0;
}

int wasmdom::domapi::nextSibling(int nodePtr)
{
    emscripten::val node = domapi::node(nodePtr);
    if (node != emscripten::val::null() && node != emscripten::val::undefined() && node["nextSibling"] != emscripten::val::null())
        return node["nextSibling"]["asmDomPtr"].as<int>();
    return 0;
}
