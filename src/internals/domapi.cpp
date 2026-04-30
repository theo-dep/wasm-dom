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

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::insertBefore(NodeId parent, NodeId newNode, NodeId ref)
{
    domQueue().emitInsertBefore(parent, newNode, ref);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeNode(NodeId node)
{
    domQueue().emitRemoveNode(node);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::appendChild(NodeId parent, NodeId child)
{
    domQueue().emitAppendChild(parent, child);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeAttribute(NodeId node, const std::string& attribute)
{
    domQueue().emitRemoveAttribute(node, attribute);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setAttribute(NodeId node, const std::string& attribute, const std::string& value)
{
    domQueue().emitSetAttribute(node, attribute, value);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setNodeValue(NodeId node, const std::string& text)
{
    domQueue().emitSetNodeValue(node, text);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setProperty(NodeId node, const std::string& name, const emscripten::val& value)
{
    domQueue().emitSetProperty(node, name, value);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::ensureEventsObject(NodeId node)
{
    domQueue().emitEnsureEventsObject(node);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::setEventsProperty(NodeId node, const std::string& name, const emscripten::val& value)
{
    domQueue().emitSetEventsProperty(node, name, value);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::deleteEventsProperty(NodeId node, const std::string& name)
{
    domQueue().emitDeleteEventsProperty(node, name);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::addEventListener(NodeId node, const std::string& event, const emscripten::val& listener)
{
    domQueue().emitAddEventListener(node, event, listener);
}

WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeEventListener(NodeId node, const std::string& event, const emscripten::val& listener)
{
    domQueue().emitRemoveEventListener(node, event, listener);
}

// Legacy val-taking entry point: still used by the public domapi test. The
// node is allocated into the JS handle table and the queue takes care of
// dropping its ref after the batch executes (the alloc gives us 1 ref, and
// emit retains a second, so the queue's drop balances the alloc; we drop
// the alloc-ref synchronously after enqueueing).
WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeNode(const emscripten::val& node)
{
    if (node.isNull() || node.isUndefined())
        return;
    const NodeId id = allocNode(node);
    domQueue().emitRemoveNode(id);
    dropNode(id);
}
