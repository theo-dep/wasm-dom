#include "domoperation.hpp"

#include "domkeys.hpp"
#include "domrecycler.hpp"
#include "jsapi.hpp"

#include <wasm-dom/conf.h>

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::enqueue(DomOperation op)
{
    _ops.push_back(std::move(op));
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomOperationQueue::flush()
{
    // Move ops out so re-entrant enqueues during flush (none expected) are safe.
    std::vector<DomOperation> ops;
    ops.swap(_ops);

    for (DomOperation& op : ops) {
        std::visit([](auto& o) {
            using T = std::decay_t<decltype(o)>;

            if constexpr (std::is_same_v<T, DomOpInsertBefore>) {
                if (o.parent.isNull() || o.parent.isUndefined())
                    return;
                jsapi::insertBefore(o.parent.as_handle(), o.node.as_handle(), o.ref.as_handle());
            } else if constexpr (std::is_same_v<T, DomOpRemoveNode>) {
                if (o.node.isNull() || o.node.isUndefined())
                    return;
                const emscripten::val parentNode{ o.node["parentNode"] };
                if (!parentNode.isNull())
                    jsapi::removeChild(parentNode.as_handle(), o.node.as_handle());
                recycler().collect(o.node);
            } else if constexpr (std::is_same_v<T, DomOpAppendChild>) {
                jsapi::appendChild(o.parent.as_handle(), o.child.as_handle());
            } else if constexpr (std::is_same_v<T, DomOpSetAttribute>) {
                if (o.name.starts_with("xml:")) {
                    jsapi::setAttributeNS(o.node.as_handle(), "http://www.w3.org/XML/1998/namespace", o.name.c_str(), o.value.c_str());
                } else if (o.name.starts_with("xlink:")) {
                    jsapi::setAttributeNS(o.node.as_handle(), "http://www.w3.org/1999/xlink", o.name.c_str(), o.value.c_str());
                } else {
                    jsapi::setAttribute(o.node.as_handle(), o.name.c_str(), o.value.c_str());
                }
            } else if constexpr (std::is_same_v<T, DomOpRemoveAttribute>) {
                jsapi::removeAttribute(o.node.as_handle(), o.name.c_str());
            } else if constexpr (std::is_same_v<T, DomOpSetNodeValue>) {
                o.node.set("nodeValue", o.value);
            } else if constexpr (std::is_same_v<T, DomOpSetProperty>) {
                o.node.set(o.name, o.value);
            } else if constexpr (std::is_same_v<T, DomOpEnsureEventsObject>) {
                if (o.node[nodeEventsKey].isUndefined()) {
                    o.node.set(nodeEventsKey, emscripten::val::object());
                }
            } else if constexpr (std::is_same_v<T, DomOpSetEventsProperty>) {
                o.node[nodeEventsKey].set(o.name, o.value);
            } else if constexpr (std::is_same_v<T, DomOpDeleteEventsProperty>) {
                o.node[nodeEventsKey].delete_(o.name);
            } else if constexpr (std::is_same_v<T, DomOpAddEventListener>) {
                jsapi::addEventListener_(o.node.as_handle(), o.event.c_str(), o.listener.as_handle());
            } else if constexpr (std::is_same_v<T, DomOpRemoveEventListener>) {
                jsapi::removeEventListener_(o.node.as_handle(), o.event.c_str(), o.listener.as_handle());
            }
        },
                   op);
    }
}

WASMDOM_SH_INLINE
wasmdom::internals::DomOperationQueue& wasmdom::internals::domQueue()
{
    static DomOperationQueue queue;
    return queue;
}
