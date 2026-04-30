#include "domoperation.hpp"

#include "handletable.hpp"
#include "jsapi.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vnode.hpp>

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::allocId()
{
    if (!_freeList.empty()) {
        const NodeId id = _freeList.back();
        _freeList.pop_back();
        return id;
    }
    return _nextId++;
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::freeId(NodeId id)
{
    if (id == nullNodeId)
        return;
    _freeList.push_back(id);
    // Drop the JS-side slot. Outside the patch hot path (called from
    // ~SharedData), so this stand-alone JS call is acceptable.
    jsapi::wdom_drop(id);
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::mount(const emscripten::val& v)
{
    if (v.isNull() || v.isUndefined())
        return nullNodeId;
    const NodeId id = allocId();
    _mounts.push_back(emscripten::val(static_cast<double>(id)));
    _mounts.push_back(v);
    return id;
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::pushString(std::string_view s, std::uint32_t& off, std::uint32_t& len)
{
    off = static_cast<std::uint32_t>(_strs.size());
    len = static_cast<std::uint32_t>(s.size());
    _strs.insert(_strs.end(), s.begin(), s.end());
}

WASMDOM_SH_INLINE
std::uint32_t wasmdom::internals::DomOperationQueue::pushVal(const emscripten::val& v)
{
    const std::uint32_t i = static_cast<std::uint32_t>(_vals.size());
    _vals.push_back(v);
    return i;
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitInsertBefore(NodeId parent, NodeId newNode, NodeId ref)
{
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
    _cmds.push_back(OP_REMOVE_NODE);
    _cmds.push_back(node);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitAppendChild(NodeId parent, NodeId child)
{
    _cmds.push_back(OP_APPEND_CHILD);
    _cmds.push_back(parent);
    _cmds.push_back(child);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitRemoveAttribute(NodeId node, std::string_view name)
{
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
    _cmds.push_back(OP_ENSURE_EVENTS_OBJECT);
    _cmds.push_back(node);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::emitSetEventsProperty(NodeId node, std::string_view name, const emscripten::val& value)
{
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
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::emitCreateElement(std::string_view tag)
{
    const NodeId id = allocId();
    std::uint32_t to, tl;
    pushString(tag, to, tl);
    _cmds.push_back(OP_CREATE_ELEMENT);
    _cmds.push_back(id);
    _cmds.push_back(to);
    _cmds.push_back(tl);
    return id;
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::emitCreateElementNS(std::string_view ns, std::string_view tag)
{
    const NodeId id = allocId();
    std::uint32_t no, nl, to, tl;
    pushString(ns, no, nl);
    pushString(tag, to, tl);
    _cmds.push_back(OP_CREATE_ELEMENT_NS);
    _cmds.push_back(id);
    _cmds.push_back(no);
    _cmds.push_back(nl);
    _cmds.push_back(to);
    _cmds.push_back(tl);
    return id;
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::emitCreateText(std::string_view text)
{
    const NodeId id = allocId();
    std::uint32_t to, tl;
    pushString(text, to, tl);
    _cmds.push_back(OP_CREATE_TEXT);
    _cmds.push_back(id);
    _cmds.push_back(to);
    _cmds.push_back(tl);
    return id;
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::emitCreateComment(std::string_view text)
{
    const NodeId id = allocId();
    std::uint32_t to, tl;
    pushString(text, to, tl);
    _cmds.push_back(OP_CREATE_COMMENT);
    _cmds.push_back(id);
    _cmds.push_back(to);
    _cmds.push_back(tl);
    return id;
}

WASMDOM_SH_INLINE
wasmdom::internals::NodeId wasmdom::internals::DomOperationQueue::emitCreateFragment()
{
    const NodeId id = allocId();
    _cmds.push_back(OP_CREATE_FRAGMENT);
    _cmds.push_back(id);
    return id;
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::flush()
{
    if (_cmds.empty() && _mounts.empty())
        return;

    emscripten::val valArr = emscripten::val::array();
    for (std::size_t i = 0; i < _vals.size(); ++i)
        valArr.set(i, _vals[i]);

    emscripten::val mountArr = emscripten::val::array();
    for (std::size_t i = 0; i < _mounts.size(); ++i)
        mountArr.set(i, _mounts[i]);

    jsapi::wdom_flush(
        _cmds.data(),
        static_cast<std::uint32_t>(_cmds.size()),
        _strs.empty() ? nullptr : _strs.data(),
        valArr.as_handle(),
        mountArr.as_handle()
    );

    _cmds.clear();
    _strs.clear();
    _vals.clear();
    _mounts.clear();
}

WASMDOM_SH_INLINE
wasmdom::internals::DomOperationQueue& wasmdom::internals::domQueue()
{
    static DomOperationQueue queue;
    return queue;
}
