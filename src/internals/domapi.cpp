#include "domapi.hpp"

#include "domoperation.hpp"
#include "handletable.hpp"
#include "jsapi.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vnode.hpp>

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createElement(const std::string& tag)
{
    return emscripten::val::take_ownership(jsapi::createElement(tag.c_str()));
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return emscripten::val::take_ownership(jsapi::createElementNS(namespaceURI.c_str(), qualifiedName.c_str()));
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createTextNode(const std::string& text)
{
    return emscripten::val::take_ownership(jsapi::createTextNode(text.c_str()));
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createComment(const std::string& comment)
{
    return emscripten::val::take_ownership(jsapi::createComment(comment.c_str()));
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::domapi::createDocumentFragment()
{
    return emscripten::val::take_ownership(jsapi::createDocumentFragment());
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

namespace wasmdom::internals::domapi
{
    using internals::nullNodeId;
    using internals::retainNode;
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::insertBefore(NodeId parent, NodeId newNode, NodeId ref)
{
    retainNode(parent);
    retainNode(newNode);
    retainNode(ref);
    domQueue().enqueue(DomOpInsertBefore{ parent, newNode, ref });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeNode(NodeId node)
{
    if (node == nullNodeId)
        return;
    retainNode(node);
    domQueue().enqueue(DomOpRemoveNode{ node });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::appendChild(NodeId parent, NodeId child)
{
    retainNode(parent);
    retainNode(child);
    domQueue().enqueue(DomOpAppendChild{ parent, child });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeAttribute(NodeId node, const std::string& attribute)
{
    retainNode(node);
    domQueue().enqueue(DomOpRemoveAttribute{ node, attribute });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setAttribute(NodeId node, const std::string& attribute, const std::string& value)
{
    retainNode(node);
    domQueue().enqueue(DomOpSetAttribute{ node, attribute, value });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setNodeValue(NodeId node, const std::string& text)
{
    retainNode(node);
    domQueue().enqueue(DomOpSetNodeValue{ node, text });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setProperty(NodeId node, const std::string& name, const emscripten::val& value)
{
    retainNode(node);
    domQueue().enqueue(DomOpSetProperty{ node, name, value });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::ensureEventsObject(NodeId node)
{
    retainNode(node);
    domQueue().enqueue(DomOpEnsureEventsObject{ node });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setEventsProperty(NodeId node, const std::string& name, const emscripten::val& value)
{
    retainNode(node);
    domQueue().enqueue(DomOpSetEventsProperty{ node, name, value });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::deleteEventsProperty(NodeId node, const std::string& name)
{
    retainNode(node);
    domQueue().enqueue(DomOpDeleteEventsProperty{ node, name });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::addEventListener(NodeId node, const std::string& event, const emscripten::val& listener)
{
    retainNode(node);
    domQueue().enqueue(DomOpAddEventListener{ node, event, listener });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeEventListener(NodeId node, const std::string& event, const emscripten::val& listener)
{
    retainNode(node);
    domQueue().enqueue(DomOpRemoveEventListener{ node, event, listener });
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeNode(const emscripten::val& node)
{
    if (node.isNull() || node.isUndefined())
        return;
    domQueue().enqueue(DomOpRemoveNode{ allocNode(node) });
}
