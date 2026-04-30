#pragma once

#include "internals/handletable.hpp"

#include <emscripten/val.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace wasmdom::internals
{
    // Single-flush DOM operation queue.
    //
    // Every mutating call from the diff pipeline encodes itself into a flat
    // command buffer (uint32_t opcodes + node ids + string offsets/lengths +
    // val indices). Strings are appended to a side blob, JS values that are
    // not DOM nodes are appended to a side vector. At flush time, a single
    // EM_JS call (`jsapi::wdom_flush`) crosses the WASM<->JS boundary and a
    // switch on the opcode dispatches to the corresponding DOM mutation.
    //
    // Each NodeId pushed into the buffer holds one extra ref on the JS-side
    // handle table (taken at emit time) so the underlying node stays alive
    // even if the owning VNode is destroyed before flush. After the JS side
    // executes the batch, the queue drops every retained id.

    enum DomOpCode : std::uint32_t
    {
        OP_INSERT_BEFORE = 0,
        OP_REMOVE_NODE = 1,
        OP_APPEND_CHILD = 2,
        OP_SET_ATTRIBUTE = 3,
        OP_REMOVE_ATTRIBUTE = 4,
        OP_SET_NODE_VALUE = 5,
        OP_SET_PROPERTY = 6,
        OP_ENSURE_EVENTS_OBJECT = 7,
        OP_SET_EVENTS_PROPERTY = 8,
        OP_DELETE_EVENTS_PROPERTY = 9,
        OP_ADD_EVENT_LISTENER = 10,
        OP_REMOVE_EVENT_LISTENER = 11,
    };

    class DomOperationQueue
    {
    public:
        // Mutators emit one opcode each. NodeIds are retained immediately;
        // they are released by the next flush().
        void emitInsertBefore(NodeId parent, NodeId newNode, NodeId ref);
        void emitRemoveNode(NodeId node);
        void emitAppendChild(NodeId parent, NodeId child);
        void emitRemoveAttribute(NodeId node, std::string_view name);
        void emitSetAttribute(NodeId node, std::string_view name, std::string_view value);
        void emitSetNodeValue(NodeId node, std::string_view text);
        void emitSetProperty(NodeId node, std::string_view name, const emscripten::val& value);
        void emitEnsureEventsObject(NodeId node);
        void emitSetEventsProperty(NodeId node, std::string_view name, const emscripten::val& value);
        void emitDeleteEventsProperty(NodeId node, std::string_view name);
        void emitAddEventListener(NodeId node, std::string_view event, const emscripten::val& listener);
        void emitRemoveEventListener(NodeId node, std::string_view event, const emscripten::val& listener);

        // Execute the recorded batch with a single JS call, then release all
        // retained NodeIds and reset internal buffers.
        void flush();

        bool empty() const { return _cmds.empty(); }

    private:
        // Append `s` to the string blob, return a pair (offset, length) ready
        // to be written into the cmd buffer.
        void pushString(std::string_view s, std::uint32_t& outOffset, std::uint32_t& outLength);
        // Append `v` to the val vector, return its index.
        std::uint32_t pushVal(const emscripten::val& v);
        // Retain `id` (no-op for nullNodeId), record it for the post-flush drop.
        void retain(NodeId id);

        std::vector<std::uint32_t> _cmds;
        std::vector<char> _strs;
        std::vector<emscripten::val> _vals;
        std::vector<NodeId> _retained;
    };

    DomOperationQueue& domQueue();
}
