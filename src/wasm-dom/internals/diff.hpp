#pragma once

#include "wasm-dom/vnode.hpp"

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

        emscripten::val& node = vnode.node();
        node.set(nodeRawsKey, emscripten::val::array());

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                node.set(key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            node[nodeRawsKey].call<void>("push", key);

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
            if (!callbacks.contains(key) && key != "ref") {
                eventKey = formatEventKey(key);
                node.call<void>("removeEventListener", eventKey, emscripten::val::module_property("eventProxy"), false);
                node[nodeEventsKey].delete_(eventKey);
            }
        }

        storeCallbacks(oldVnode, vnode);
        if (node[nodeEventsKey].isUndefined()) {
            node.set(nodeEventsKey, emscripten::val::object());
        }

        for (const auto& [key, _] : callbacks) {
            if (!oldCallbacks.contains(key) && key != "ref") {
                eventKey = formatEventKey(key);
                node.call<void>("addEventListener", eventKey, emscripten::val::module_property("eventProxy"), false);
                node[nodeEventsKey].set(eventKey, emscripten::val::module_property("eventProxy"));
            }
        }

        const Callback callback = vnode.hash() & hasRef ? callbacks.at("ref") : Callback();
        const Callback oldCallback = oldVnode.hash() & hasRef ? oldCallbacks.at("ref") : Callback();

        // callback store a function pointer and it is the same, do nothing
        const auto rawCallback = callback.target<bool (*)(emscripten::val)>();
        const auto rawOldCallback = oldCallback.target<bool (*)(emscripten::val)>();
        if (rawCallback && rawOldCallback && *rawCallback == *rawOldCallback)
            return;

        if (oldCallback) {
            oldCallback(emscripten::val::null());
        }

        if (callback) {
            callback(node);
        }
    }
}
