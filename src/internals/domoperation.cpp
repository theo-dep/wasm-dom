#include "domoperation.hpp"

#include "domkeys.hpp"
#include "handletable.hpp"
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
                const emscripten::val parent = resolveNode(o.parent);
                const emscripten::val node = resolveNode(o.node);
                const emscripten::val ref = resolveNode(o.ref);
                if (!parent.isNull() && !parent.isUndefined())
                    jsapi::insertBefore(parent.as_handle(), node.as_handle(), ref.as_handle());
                dropNode(o.parent);
                dropNode(o.node);
                dropNode(o.ref);
            } else if constexpr (std::is_same_v<T, DomOpRemoveNode>) {
                const emscripten::val node = resolveNode(o.node);
                if (!node.isNull() && !node.isUndefined()) {
                    const emscripten::val parentNode{ node["parentNode"] };
                    if (!parentNode.isNull())
                        jsapi::removeChild(parentNode.as_handle(), node.as_handle());
                }
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpAppendChild>) {
                const emscripten::val parent = resolveNode(o.parent);
                const emscripten::val child = resolveNode(o.child);
                jsapi::appendChild(parent.as_handle(), child.as_handle());
                dropNode(o.parent);
                dropNode(o.child);
            } else if constexpr (std::is_same_v<T, DomOpSetAttribute>) {
                const emscripten::val node = resolveNode(o.node);
                if (o.name.starts_with("xml:")) {
                    jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/XML/1998/namespace", o.name.c_str(), o.value.c_str());
                } else if (o.name.starts_with("xlink:")) {
                    jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/1999/xlink", o.name.c_str(), o.value.c_str());
                } else {
                    jsapi::setAttribute(node.as_handle(), o.name.c_str(), o.value.c_str());
                }
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpRemoveAttribute>) {
                const emscripten::val node = resolveNode(o.node);
                jsapi::removeAttribute(node.as_handle(), o.name.c_str());
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpSetNodeValue>) {
                emscripten::val node = resolveNode(o.node);
                node.set("nodeValue", o.value);
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpSetProperty>) {
                emscripten::val node = resolveNode(o.node);
                node.set(o.name, o.value);
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpEnsureEventsObject>) {
                emscripten::val node = resolveNode(o.node);
                if (node[nodeEventsKey].isUndefined()) {
                    node.set(nodeEventsKey, emscripten::val::object());
                }
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpSetEventsProperty>) {
                emscripten::val node = resolveNode(o.node);
                node[nodeEventsKey].set(o.name, o.value);
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpDeleteEventsProperty>) {
                emscripten::val node = resolveNode(o.node);
                node[nodeEventsKey].delete_(o.name);
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpAddEventListener>) {
                const emscripten::val node = resolveNode(o.node);
                jsapi::addEventListener_(node.as_handle(), o.event.c_str(), o.listener.as_handle());
                dropNode(o.node);
            } else if constexpr (std::is_same_v<T, DomOpRemoveEventListener>) {
                const emscripten::val node = resolveNode(o.node);
                jsapi::removeEventListener_(node.as_handle(), o.event.c_str(), o.listener.as_handle());
                dropNode(o.node);
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
