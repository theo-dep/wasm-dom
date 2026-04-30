#include "domoperation.hpp"

#include "handletable.hpp"
#include "jsapi.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vnode.hpp>

#include <cstdint>

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::pushString(std::string_view s, std::uint32_t& outOffset, std::uint32_t& outLength)
{
    outOffset = static_cast<std::uint32_t>(_strs.size());
    outLength = static_cast<std::uint32_t>(s.size());
    _strs.insert(_strs.end(), s.begin(), s.end());
}

WASMDOM_SH_INLINE
std::uint32_t wasmdom::internals::DomOperationQueue::pushVal(const emscripten::val& v)
{
    const std::uint32_t idx = static_cast<std::uint32_t>(_vals.size());
    _vals.push_back(v);
    return idx;
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::retain(NodeId id)
{
    retainNode(id);
    _retained.push_back(id);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitInsertBefore(NodeId parent, NodeId newNode, NodeId ref)
{
    retain(parent);
    retain(newNode);
    retain(ref);
    _cmds.push_back(OP_INSERT_BEFORE);
    _cmds.push_back(parent);
    _cmds.push_back(newNode);
    _cmds.push_back(ref);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitRemoveNode(NodeId node)
{
    if (node == nullNodeId)
        return;
    retain(node);
    _cmds.push_back(OP_REMOVE_NODE);
    _cmds.push_back(node);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitAppendChild(NodeId parent, NodeId child)
{
    retain(parent);
    retain(child);
    _cmds.push_back(OP_APPEND_CHILD);
    _cmds.push_back(parent);
    _cmds.push_back(child);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitRemoveAttribute(NodeId node, std::string_view name)
{
    retain(node);
    std::uint32_t no, nl;
    pushString(name, no, nl);
    _cmds.push_back(OP_REMOVE_ATTRIBUTE);
    _cmds.push_back(node);
    _cmds.push_back(no);
    _cmds.push_back(nl);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitSetAttribute(NodeId node, std::string_view name, std::string_view value)
{
    retain(node);
    std::uint32_t no, nl, vo, vl;
    pushString(name, no, nl);
    pushString(value, vo, vl);
    _cmds.push_back(OP_SET_ATTRIBUTE);
    _cmds.push_back(node);
    _cmds.push_back(no);
    _cmds.push_back(nl);
    _cmds.push_back(vo);
    _cmds.push_back(vl);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitSetNodeValue(NodeId node, std::string_view text)
{
    retain(node);
    std::uint32_t vo, vl;
    pushString(text, vo, vl);
    _cmds.push_back(OP_SET_NODE_VALUE);
    _cmds.push_back(node);
    _cmds.push_back(vo);
    _cmds.push_back(vl);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitSetProperty(NodeId node, std::string_view name, const emscripten::val& value)
{
    retain(node);
    std::uint32_t no, nl;
    pushString(name, no, nl);
    const std::uint32_t vi = pushVal(value);
    _cmds.push_back(OP_SET_PROPERTY);
    _cmds.push_back(node);
    _cmds.push_back(no);
    _cmds.push_back(nl);
    _cmds.push_back(vi);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitEnsureEventsObject(NodeId node)
{
    retain(node);
    _cmds.push_back(OP_ENSURE_EVENTS_OBJECT);
    _cmds.push_back(node);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitSetEventsProperty(NodeId node, std::string_view name, const emscripten::val& value)
{
    retain(node);
    std::uint32_t no, nl;
    pushString(name, no, nl);
    const std::uint32_t vi = pushVal(value);
    _cmds.push_back(OP_SET_EVENTS_PROPERTY);
    _cmds.push_back(node);
    _cmds.push_back(no);
    _cmds.push_back(nl);
    _cmds.push_back(vi);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitDeleteEventsProperty(NodeId node, std::string_view name)
{
    retain(node);
    std::uint32_t no, nl;
    pushString(name, no, nl);
    _cmds.push_back(OP_DELETE_EVENTS_PROPERTY);
    _cmds.push_back(node);
    _cmds.push_back(no);
    _cmds.push_back(nl);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitAddEventListener(NodeId node, std::string_view event, const emscripten::val& listener)
{
    retain(node);
    std::uint32_t eo, el;
    pushString(event, eo, el);
    const std::uint32_t vi = pushVal(listener);
    _cmds.push_back(OP_ADD_EVENT_LISTENER);
    _cmds.push_back(node);
    _cmds.push_back(eo);
    _cmds.push_back(el);
    _cmds.push_back(vi);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitRemoveEventListener(NodeId node, std::string_view event, const emscripten::val& listener)
{
    retain(node);
    std::uint32_t eo, el;
    pushString(event, eo, el);
    const std::uint32_t vi = pushVal(listener);
    _cmds.push_back(OP_REMOVE_EVENT_LISTENER);
    _cmds.push_back(node);
    _cmds.push_back(eo);
    _cmds.push_back(el);
    _cmds.push_back(vi);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::flush()
{
    if (_cmds.empty())
        return;

    // Pack vals into a JS Array so the handler can index them directly.
    emscripten::val valArr = emscripten::val::array();
    for (std::size_t i = 0; i < _vals.size(); ++i)
        valArr.set(i, _vals[i]);

    jsapi::wdom_flush(
        _cmds.data(),
        static_cast<std::uint32_t>(_cmds.size()),
        _strs.empty() ? nullptr : _strs.data(),
        valArr.as_handle()
    );

    for (NodeId id : _retained)
        dropNode(id);

    _cmds.clear();
    _strs.clear();
    _vals.clear();
    _retained.clear();
}

WASMDOM_SH_INLINE
wasmdom::internals::DomOperationQueue& wasmdom::internals::domQueue()
{
    static DomOperationQueue queue;
    return queue;
}
