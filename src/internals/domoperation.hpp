#pragma once

#include <emscripten/val.h>

#include <string>
#include <variant>
#include <vector>

namespace wasmdom::internals
{
    // DOM mutation operations are recorded into a queue during a patch and
    // executed in batch at the end. Read-only operations (createElement, ...)
    // remain synchronous because their result is needed by subsequent ops.

    struct DomOpInsertBefore
    {
        emscripten::val parent;
        emscripten::val node;
        emscripten::val ref;
    };

    struct DomOpRemoveNode
    {
        emscripten::val node;
    };

    struct DomOpAppendChild
    {
        emscripten::val parent;
        emscripten::val child;
    };

    struct DomOpSetAttribute
    {
        emscripten::val node;
        std::string name;
        std::string value;
    };

    struct DomOpRemoveAttribute
    {
        emscripten::val node;
        std::string name;
    };

    struct DomOpSetNodeValue
    {
        emscripten::val node;
        std::string value;
    };

    struct DomOpSetProperty
    {
        emscripten::val node;
        std::string name;
        emscripten::val value;
    };

    struct DomOpEnsureEventsObject
    {
        emscripten::val node;
    };

    struct DomOpSetEventsProperty
    {
        emscripten::val node;
        std::string name;
        emscripten::val value;
    };

    struct DomOpDeleteEventsProperty
    {
        emscripten::val node;
        std::string name;
    };

    struct DomOpAddEventListener
    {
        emscripten::val node;
        std::string event;
        emscripten::val listener;
    };

    struct DomOpRemoveEventListener
    {
        emscripten::val node;
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
