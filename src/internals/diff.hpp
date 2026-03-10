#pragma once

#include "internals/domapi.hpp"
#include "internals/domkeys.hpp"
#include "internals/jsapi.hpp"
#include "internals/wire.hpp"

#include <wasm-dom/vnodedata.hpp>

#include <ranges>
#include <unordered_map>

namespace wasmdom::internals
{
    inline void diffAttrs(const VNodeData& oldVnode, const VNodeData& vnode)
    {
        const Attrs& oldAttrs = oldVnode.attrs;
        const Attrs& attrs = vnode.attrs;

        const emscripten::val& node = vnode.data.node;

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

    inline void diffProps(const VNodeData& oldVnode, VNodeData& vnode)
    {
        const Props& oldProps = oldVnode.props;
        const Props& props = vnode.props;

        const emscripten::val nodeRaws = emscripten::val::array(
            props | std::views::keys | std::ranges::to<std::vector<std::string>>()
        );

        emscripten::val& node = vnode.data.node;
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

    inline void diffCallbacks(const VNodeData& oldVnode, VNodeData& vnode)
    {
        const Callbacks& oldCallbacks = oldVnode.callbacks;
        const Callbacks& callbacks = vnode.callbacks;

        emscripten::val& node = vnode.data.node;

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            eventKey = formatEventKey(key);
            jsapi::removeEventListener_(node.as_handle(), eventKey.c_str(), node[nodeEventsKey][eventKey].as_handle());
            node[nodeEventsKey].delete_(eventKey);
        }

        if (node[nodeEventsKey].isUndefined()) {
            node.set(nodeEventsKey, emscripten::val::object());
        }

        for (auto& [key, val] : callbacks) {
            eventKey = formatEventKey(key);
            const emscripten::val jsCallback = toJsCallback(val);
            const emscripten::val functorAdapter = jsCallback["opcall"].call<emscripten::val>("bind", jsCallback);
            jsapi::addEventListener_(node.as_handle(), eventKey.c_str(), functorAdapter.as_handle());
            node[nodeEventsKey].set(eventKey, functorAdapter);
        }
    }

    inline void diff(const VNodeData& oldVnode, const VNodeData& vnode)
    {
        const std::size_t vnodes = vnode.hash | oldVnode.hash;

        if (vnodes & hasAttrs) {
            diffAttrs(oldVnode, vnode);
        }
        if (vnodes & hasProps) {
            diffProps(oldVnode, vnode);
        }
        if (vnodes & hasCallbacks) {
            diffCallbacks(oldVnode, vnode);
        }
    }
}
