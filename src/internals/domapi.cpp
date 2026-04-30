#include "domapi.hpp"

#include "domoperation.hpp"
#include "domrecycler.hpp"
#include "jsapi.hpp"

#include <wasm-dom/conf.h>

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
    domQueue().enqueue(DomOpInsertBefore{ parentNode, newNode, referenceNode });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeNode(const emscripten::val& node)
{
    if (node.isNull() || node.isUndefined())
        return;
    domQueue().enqueue(DomOpRemoveNode{ node });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::appendChild(const emscripten::val& parent, const emscripten::val& child)
{
    domQueue().enqueue(DomOpAppendChild{ parent, child });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeAttribute(const emscripten::val& node, const std::string& attribute)
{
    domQueue().enqueue(DomOpRemoveAttribute{ node, attribute });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value)
{
    domQueue().enqueue(DomOpSetAttribute{ node, attribute, value });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setNodeValue(const emscripten::val& node, const std::string& text)
{
    domQueue().enqueue(DomOpSetNodeValue{ node, text });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setProperty(const emscripten::val& node, const std::string& name, const emscripten::val& value)
{
    domQueue().enqueue(DomOpSetProperty{ node, name, value });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::ensureEventsObject(const emscripten::val& node)
{
    domQueue().enqueue(DomOpEnsureEventsObject{ node });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setEventsProperty(const emscripten::val& node, const std::string& name, const emscripten::val& value)
{
    domQueue().enqueue(DomOpSetEventsProperty{ node, name, value });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::deleteEventsProperty(const emscripten::val& node, const std::string& name)
{
    domQueue().enqueue(DomOpDeleteEventsProperty{ node, name });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::addEventListener(const emscripten::val& node, const std::string& event, const emscripten::val& listener)
{
    domQueue().enqueue(DomOpAddEventListener{ node, event, listener });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeEventListener(const emscripten::val& node, const std::string& event, const emscripten::val& listener)
{
    domQueue().enqueue(DomOpRemoveEventListener{ node, event, listener });
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::parentNode(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined())
        return node["parentNode"];
    return emscripten::val::null();
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::nextSibling(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined())
        return node["nextSibling"];
    return emscripten::val::null();
}
