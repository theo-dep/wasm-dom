#include "domapi.hpp"

#include "domrecycler.hpp"
#include "jsapi.hpp"

#include <wasm-dom/conf.h>

namespace wasmdom::internals
{
    inline DomRecycler& recycler()
    {
        static DomRecycler recycler(true);
        return recycler;
    }
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createElement(const std::string& tag)
{
    return recycler().create(tag);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return recycler().createNS(qualifiedName, namespaceURI);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createTextNode(const std::string& text)
{
    return recycler().createText(text);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createComment(const std::string& comment)
{
    return recycler().createComment(comment);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createDocumentFragment()
{
    return emscripten::val::take_ownership(jsapi::createDocumentFragment());
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode)
{
    if (parentNode.isNull() || parentNode.isUndefined())
        return;

    jsapi::insertBefore(parentNode.as_handle(), newNode.as_handle(), referenceNode.as_handle());
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeChild(const emscripten::val& child)
{
    if (child.isNull() || child.isUndefined())
        return;

    const emscripten::val parentNode(child["parentNode"]);
    if (!parentNode.isNull())
        jsapi::removeChild(parentNode.as_handle(), child.as_handle());

    recycler().collect(child);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::appendChild(const emscripten::val& parent, const emscripten::val& child)
{
    jsapi::appendChild(parent.as_handle(), child.as_handle());
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeAttribute(const emscripten::val& node, const std::string& attribute)
{
    jsapi::removeAttribute(node.as_handle(), attribute.c_str());
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value)
{
    if (attribute.starts_with("xml:")) {
        jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/XML/1998/namespace", attribute.c_str(), value.c_str());
    } else if (attribute.starts_with("xlink:")) {
        jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/1999/xlink", attribute.c_str(), value.c_str());
    } else {
        jsapi::setAttribute(node.as_handle(), attribute.c_str(), value.c_str());
    }
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setNodeValue(emscripten::val& node, const std::string& text)
{
    node.set("nodeValue", text);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::parentNode(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined() && !node["parentNode"].isNull())
        return node["parentNode"];
    return emscripten::val::null();
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::nextSibling(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined() && !node["nextSibling"].isNull())
        return node["nextSibling"];
    return emscripten::val::null();
}
