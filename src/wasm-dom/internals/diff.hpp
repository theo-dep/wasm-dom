#pragma once

#include "wasm-dom/attribute.hpp"
#include "wasm-dom/domapi.hpp"
#include "wasm-dom/domkeys.hpp"
#include "wasm-dom/internals/jsapi.hpp"
#include "wasm-dom/vnode.hpp"

#include <emscripten/bind.h>

#include <ranges>
#include <unordered_map>

namespace wasmdom::internals
{
    inline void diffAttrs(const VNode& oldVnode, const VNode& vnode)
    {
        const Attrs& oldAttrs = oldVnode.attrs();
        const Attrs& attrs = vnode.attrs();

        const emscripten::val& node = vnode.node();

        for (const auto& [key, _] : oldAttrs) {
            if (!attrs.contains(key)) {
                domapi::removeAttribute(node, key);
            }
        }

        for (const auto& [key, val] : attrs) {
            const auto oldAttrsIt = oldAttrs.find(key);
            if (oldAttrsIt == oldAttrs.cend() || oldAttrsIt->second != val) {
                domapi::setAttribute(node, key, val);
            }
        }
    }

    inline void diffProps(const VNode& oldVnode, VNode& vnode)
    {
        const Props& oldProps = oldVnode.props();
        const Props& props = vnode.props();

        const emscripten::val nodeRaws = emscripten::val::array(
            props | std::views::keys | std::ranges::to<std::vector<std::string>>()
        );

        emscripten::val& node = vnode.node();
        node.set(nodeRawsKey, nodeRaws);

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                node.set(key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            const auto oldPropsIt = oldProps.find(key);
            if (oldPropsIt == oldProps.cend() ||
                !val.strictlyEquals(oldPropsIt->second) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(node[key]))) {
                node.set(key, val);
            }
        }
    }

    inline std::string formatEventKey(const std::string& key)
    {
        static constexpr std::string_view eventPrefix = "on";
        if (key.starts_with(eventPrefix))
            return key.substr(eventPrefix.size());
        return key;
    }

    inline void diffCallbacks(const VNode& oldVnode, VNode& vnode)
    {
        const Callbacks& oldCallbacks = oldVnode.callbacks();
        const Callbacks& callbacks = vnode.callbacks();

        emscripten::val& node = vnode.node();

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            if (!callbacks.contains(key)) {
                eventKey = formatEventKey(key);
                jsapi::removeEventListener_(node.as_handle(), eventKey.c_str(), emscripten::val::module_property(nodeEventProxyKey).as_handle());
                node[nodeEventsKey].delete_(eventKey);
            }
        }

        if (node[nodeEventsKey].isUndefined()) {
            node.set(nodeEventsKey, emscripten::val::object());
        }

        for (const auto& [key, value] : callbacks) {
            // can't compare callbacks, add all
            eventKey = formatEventKey(key);
            jsapi::addEventListener_(node.as_handle(), eventKey.c_str(), emscripten::val::module_property(nodeEventProxyKey).as_handle());
            node[nodeEventsKey].set(eventKey, value);
        }
    }

    inline bool eventProxy(emscripten::val event)
    {
        const emscripten::val callbacks = event["currentTarget"][nodeEventsKey];
        return callbacks[event["type"]].as<Callback>()(event);
    }

    // in single header mode, the binding function must be registered only once
    // see https://github.com/emscripten-core/emscripten/issues/25219
    __attribute__((weak)) emscripten::internal::InitFunc wasmdomInitEventProxyFunc([] {
        emscripten::class_<Callback>("Callback");
        emscripten::function(nodeEventProxyKey, eventProxy);
    });

}
