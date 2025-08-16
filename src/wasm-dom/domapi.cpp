#include "domapi.hpp"

#include "domrecycler.hpp"
#include "internals/jsapi.hpp"

namespace wasmdom::internals
{
    inline DomRecycler& recycler()
    {
        static DomRecycler recycler(true);
        return recycler;
    }
}

emscripten::val wasmdom::domapi::createElement(const std::string& tag)
{
    return internals::recycler().create(tag);
}

emscripten::val wasmdom::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return internals::recycler().createNS(qualifiedName, namespaceURI);
}

emscripten::val wasmdom::domapi::createTextNode(const std::string& text)
{
    return internals::recycler().createText(text);
}

emscripten::val wasmdom::domapi::createComment(const std::string& comment)
{
    return internals::recycler().createComment(comment);
}

emscripten::val wasmdom::domapi::createDocumentFragment()
{
    return emscripten::val::take_ownership(internals::jsapi::createDocumentFragment());
}

void wasmdom::domapi::insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode)
{
    if (parentNode.isNull() || parentNode.isUndefined())
        return;

    internals::jsapi::insertBefore(parentNode.as_handle(), newNode.as_handle(), referenceNode.as_handle());
}

void wasmdom::domapi::removeChild(const emscripten::val& child)
{
    if (child.isNull() || child.isUndefined())
        return;

    const emscripten::val parentNode(child["parentNode"]);
    if (!parentNode.isNull())
        internals::jsapi::removeChild(parentNode.as_handle(), child.as_handle());

    internals::recycler().collect(child);
}

void wasmdom::domapi::appendChild(const emscripten::val& parent, const emscripten::val& child)
{
    internals::jsapi::appendChild(parent.as_handle(), child.as_handle());
}

void wasmdom::domapi::removeAttribute(const emscripten::val& node, const std::string& attribute)
{
    internals::jsapi::removeAttribute(node.as_handle(), attribute.c_str());
}

void wasmdom::domapi::setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value)
{
    if (attribute.starts_with("xml:")) {
        internals::jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/XML/1998/namespace", attribute.c_str(), value.c_str());
    } else if (attribute.starts_with("xlink:")) {
        internals::jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/1999/xlink", attribute.c_str(), value.c_str());
    } else {
        internals::jsapi::setAttribute(node.as_handle(), attribute.c_str(), value.c_str());
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
