#pragma once

#include "internals/handletable.hpp"

#include <emscripten/val.h>

#include <string>
#include <variant>
#include <vector>

namespace wasmdom::internals
{
    // DOM mutation operations are recorded into a queue during a patch and
    // executed in batch at the end. Read-only operations (createElement, ...)
    // remain synchronous because their result is needed by subsequent ops.
    //
    // Node arguments are stored as NodeIds: uint32 indices into a JS-side
    // refcounted handle table. Allocation happens at enqueue time, the slot
    // is released after the op is executed during flush.
    // JS values that are not DOM nodes (property values, event listeners)
    // continue to be carried as emscripten::val.

    struct DomOpInsertBefore
    {
        NodeId parent;
        NodeId node;
        NodeId ref;
    };

    struct DomOpRemoveNode
    {
        NodeId node;
    };

    struct DomOpAppendChild
    {
        NodeId parent;
        NodeId child;
    };

    struct DomOpSetAttribute
    {
        NodeId node;
        std::string name;
        std::string value;
    };

    struct DomOpRemoveAttribute
    {
        NodeId node;
        std::string name;
    };

    struct DomOpSetNodeValue
    {
        NodeId node;
        std::string value;
    };

    struct DomOpSetProperty
    {
        NodeId node;
        std::string name;
        emscripten::val value;
    };

    struct DomOpEnsureEventsObject
    {
        NodeId node;
    };

    struct DomOpSetEventsProperty
    {
        NodeId node;
        std::string name;
        emscripten::val value;
    };

    struct DomOpDeleteEventsProperty
    {
        NodeId node;
        std::string name;
    };

    struct DomOpAddEventListener
    {
        NodeId node;
        std::string event;
        emscripten::val listener;
    };

    struct DomOpRemoveEventListener
    {
        NodeId node;
        std::string event;
        emscripten::val listener;
    };

    using DomOperation = std::variant<
        DomOpInsertBefore,
        DomOpRemoveNode,
        DomOpAppendChild,
        DomOpSetAttribute,
        DomOpRemoveAttribute,
        DomOpSetNodeValue,
        DomOpSetProperty,
        DomOpEnsureEventsObject,
        DomOpSetEventsProperty,
        DomOpDeleteEventsProperty,
        DomOpAddEventListener,
        DomOpRemoveEventListener>;

    class DomOperationQueue
    {
    public:
        void enqueue(DomOperation op);
        void flush();
        bool empty() const { return _ops.empty(); }
        std::size_t size() const { return _ops.size(); }

    private:
        std::vector<DomOperation> _ops;
    };

    DomOperationQueue& domQueue();
}
