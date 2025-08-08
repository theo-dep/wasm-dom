#include "domapi.hpp"

#include "vnode.hpp"

#include <emscripten.h>

namespace wasmdom::domapi
{
    int addPtr(const emscripten::val& node)
    {
        static int lastPtr = 0;

        if (node == emscripten::val::null() || node == emscripten::val::undefined())
            return 0;
        if (node["asmDomPtr"] != emscripten::val::undefined())
            return node["asmDomPtr"].as<int>();

        emscripten::val nodes = emscripten::val::module_property("nodes");
        emscripten::val newNode = node;

        ++lastPtr;
        newNode.set("asmDomPtr", lastPtr);
        nodes.set(lastPtr, newNode);
        return lastPtr;
    }
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

    emscripten::val nodes = emscripten::val::module_property("nodes");
    nodes[parentNodePtr].call<void>("insertBefore", nodes[newNodePtr], nodes[referenceNodePtr]);
}

void wasmdom::domapi::removeChild(int childPtr)
{
    emscripten::val nodes = emscripten::val::module_property("nodes");
    emscripten::val node = nodes[childPtr];
    if (node == emscripten::val::null() || node == emscripten::val::undefined())
        return;

    emscripten::val parentNode = node["parentNode"];
    if (parentNode != emscripten::val::null())
        parentNode.call<void>("removeChild", node);

    emscripten::val::module_property("recycler").call<void>("collect", node);
}

void wasmdom::domapi::appendChild(int parentPtr, int childPtr)
{
    emscripten::val nodes = emscripten::val::module_property("nodes");
    nodes[parentPtr].call<void>("appendChild", nodes[childPtr]);
}

void wasmdom::domapi::removeAttribute(int nodePtr, const std::string& attribute)
{
    emscripten::val nodes = emscripten::val::module_property("nodes");
    nodes[nodePtr].call<void>("removeAttribute", attribute);
}

void wasmdom::domapi::setAttribute(int nodePtr, const std::string& attribute, const std::string& value)
{
    emscripten::val nodes = emscripten::val::module_property("nodes");
    emscripten::val node = nodes[nodePtr];

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
    emscripten::val nodes = emscripten::val::module_property("nodes");
    nodes[nodePtr].set("nodeValue", text);
}

int wasmdom::domapi::parentNode(int nodePtr)
{
    emscripten::val nodes = emscripten::val::module_property("nodes");
    emscripten::val node = nodes[nodePtr];

    if (node != emscripten::val::null() && node != emscripten::val::undefined() && node["parentNode"] != emscripten::val::null())
        return node["parentNode"]["asmDomPtr"].as<int>();
    return 0;
}

int wasmdom::domapi::nextSibling(int nodePtr)
{
    emscripten::val nodes = emscripten::val::module_property("nodes");
    emscripten::val node = nodes[nodePtr];

    if (node != emscripten::val::null() && node != emscripten::val::undefined() && node["nextSibling"] != emscripten::val::null())
        return node["nextSibling"]["asmDomPtr"].as<int>();
    return 0;
}
