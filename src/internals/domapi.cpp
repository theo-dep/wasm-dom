#include "domapi.hpp"

#include "domoperation.hpp"
#include "handletable.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vnode.hpp>

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::domapi::createElement(const std::string& tag)
{
    return domQueue().emitCreateElement(tag);
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return domQueue().emitCreateElementNS(namespaceURI, qualifiedName);
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::domapi::createTextNode(const std::string& text)
{
    return domQueue().emitCreateText(text);
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::domapi::createComment(const std::string& comment)
{
    return domQueue().emitCreateComment(comment);
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::domapi::createDocumentFragment()
{
    return domQueue().emitCreateFragment();
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

// Legacy val-taking entry point kept for the public domapi test. Mounts the
// externally-owned node into the handle table, emits removeNode, then frees
// the id (the JS table slot will be cleared at flush via wdom_drop side
// effects on freeId).
WASMDOM_SH_INLINE
void wasmdom::internals::domapi::removeNode(const emscripten::val& node)
{
    if (node.isNull() || node.isUndefined())
        return;
    const NodeId id = domQueue().mount(node);
    domQueue().emitRemoveNode(id);
    // Free after flush would be cleanest but we don't have a hook. Defer:
    // since freeId would call wdom_drop synchronously and we want the node
    // to still exist in the table when wdom_flush runs, keep the slot in
    // the handle table by NOT calling freeId here.
}
