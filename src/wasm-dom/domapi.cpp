#include "domapi.hpp"

#include "domrecycler.hpp"
#include "vnode.hpp"

#include <emscripten.h>

#include <unordered_map>

namespace wasmdom
{

    static inline DomRecycler& recycler()
    {
        static DomRecycler recycler(true);
        return recycler;
    }

}

emscripten::val wasmdom::domapi::createElement(const std::string& tag)
{
    return recycler().create(tag);
}

emscripten::val wasmdom::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return recycler().createNS(qualifiedName, namespaceURI);
}

emscripten::val wasmdom::domapi::createTextNode(const std::string& text)
{
    return recycler().createText(text);
}

emscripten::val wasmdom::domapi::createComment(const std::string& comment)
{
    return recycler().createComment(comment);
}

emscripten::val wasmdom::domapi::createDocumentFragment()
{
    return emscripten::val::global("document").call<emscripten::val>("createDocumentFragment");
}

void wasmdom::domapi::insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode)
{
    if (parentNode.isNull() || parentNode.isUndefined())
        return;

    parentNode.call<void>("insertBefore", newNode, referenceNode);
}

void wasmdom::domapi::removeChild(const emscripten::val& child)
{
    if (child.isNull() || child.isUndefined())
        return;

    const emscripten::val parentNode(child["parentNode"]);
    if (!parentNode.isNull())
        parentNode.call<void>("removeChild", child);

    recycler().collect(child);
}

void wasmdom::domapi::appendChild(const emscripten::val& parent, const emscripten::val& child)
{
    parent.call<void>("appendChild", child);
}

void wasmdom::domapi::removeAttribute(const emscripten::val& node, const std::string& attribute)
{
    node.call<void>("removeAttribute", attribute);
}

void wasmdom::domapi::setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value)
{
    if (attribute.starts_with("xml:")) {
        node.call<void>("setAttributeNS", std::string("http://www.w3.org/XML/1998/namespace"), attribute, value);
    } else if (attribute.starts_with("xlink:")) {
        node.call<void>("setAttributeNS", std::string("http://www.w3.org/1999/xlink"), attribute, value);
    } else {
        node.call<void>("setAttribute", attribute, value);
    }
}

void wasmdom::domapi::setNodeValue(emscripten::val& node, const std::string& text)
{
    node.set("nodeValue", text);
}

emscripten::val wasmdom::domapi::parentNode(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined() && !node["parentNode"].isNull())
        return node["parentNode"];
    return emscripten::val::null();
}

emscripten::val wasmdom::domapi::nextSibling(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined() && !node["nextSibling"].isNull())
        return node["nextSibling"];
    return emscripten::val::null();
}
