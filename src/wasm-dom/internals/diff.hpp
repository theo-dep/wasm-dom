#pragma once

#include "wasm-dom/internals/jsapi.hpp"
#include "wasm-dom/vnode.hpp"

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

    // store callbacks addresses to be called in functionCallback
    inline std::unordered_map<std::size_t, Callbacks>& vnodeCallbacks()
    {
        static std::unordered_map<std::size_t, Callbacks> vnodeCallbacks;
        return vnodeCallbacks;
    }

    inline std::size_t incrementCallbackKey()
    {
        static std::size_t lastCallbackKey = 0;
        return ++lastCallbackKey;
    }

    inline void storeCallbacks(const VNode& oldVnode, VNode& vnode)
    {
        const emscripten::val& oldNode = oldVnode.node();

        std::size_t callbackKey = 0;
        if (oldNode.isNull()) {
            callbackKey = incrementCallbackKey();
        } else {
            const emscripten::val oldNodeCallbacksKey = oldNode[nodeCallbacksKey];
            if (oldNodeCallbacksKey.isUndefined()) {
                callbackKey = incrementCallbackKey();
            } else {
                callbackKey = oldNodeCallbacksKey.as<std::size_t>();
            }
        }

        emscripten::val& node = vnode.node();
        node.set(nodeCallbacksKey, callbackKey);
        vnodeCallbacks()[callbackKey] = vnode.callbacks();
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
                jsapi::removeEventListener_(node.as_handle(), eventKey.c_str(), emscripten::val::module_property("eventProxy").as_handle());
                node[nodeEventsKey].delete_(eventKey);
            }
        }

        storeCallbacks(oldVnode, vnode);
        if (node[nodeEventsKey].isUndefined()) {
            node.set(nodeEventsKey, emscripten::val::object());
        }

        for (const auto& [key, _] : callbacks) {
            if (!oldCallbacks.contains(key)) {
                eventKey = formatEventKey(key);
                jsapi::addEventListener_(node.as_handle(), eventKey.c_str(), emscripten::val::module_property("eventProxy").as_handle());
                node[nodeEventsKey].set(eventKey, emscripten::val::module_property("eventProxy"));
            }
        }
    }
}
