#pragma once

#include "internals/handletable.hpp"

#include <emscripten/val.h>

#include <cstdint>
#include <string_view>
#include <vector>

namespace wasmdom::internals
{
    // Opcodes mirrored on the JS side (see jsapi.c / wdom_flush). Keep in sync.
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
        OP_CREATE_ELEMENT = 12,
        OP_CREATE_ELEMENT_NS = 13,
        OP_CREATE_TEXT = 14,
        OP_CREATE_COMMENT = 15,
        OP_CREATE_FRAGMENT = 16,
    };

    // Allocator + reusable queue. NodeId allocation is done in C++ so that
    // the patch pipeline never crosses the WASM<->JS boundary except through
    // the single wdom_flush call (and the rare wdom_get / wdom_drop calls
    // outside the patch hot path).
    class DomOperationQueue
    {
    public:
        // Allocate a fresh NodeId from the C++-side freelist.
        NodeId allocId();
        // Return a NodeId to the freelist.
        void freeId(NodeId id);

        // Register an externally-owned node (e.g. mounted via toVNode). The
        // (id, val) pair is sent to JS at flush time so the table is
        // populated before any opcode references the id.
        NodeId mount(const emscripten::val& v);

        // Mutators.
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

        // Creation opcodes. Allocate an id and emit the matching CREATE_* op.
        NodeId emitCreateElement(std::string_view tag);
        NodeId emitCreateElementNS(std::string_view ns, std::string_view tag);
        NodeId emitCreateText(std::string_view text);
        NodeId emitCreateComment(std::string_view text);
        NodeId emitCreateFragment();

        void flush();
        bool empty() const { return _cmds.empty(); }

    private:
        void pushString(std::string_view s, std::uint32_t& off, std::uint32_t& len);
        std::uint32_t pushVal(const emscripten::val& v);

        std::vector<std::uint32_t> _cmds;
        std::vector<char> _strs;
        std::vector<emscripten::val> _vals;
        // Flat (id, val) pairs for OP_MOUNT-equivalent registration before
        // opcode dispatch.
        std::vector<emscripten::val> _mounts;

        NodeId _nextId{ 1 }; // 0 is reserved for nullNodeId
        std::vector<NodeId> _freeList;
    };

    DomOperationQueue& domQueue();
}
