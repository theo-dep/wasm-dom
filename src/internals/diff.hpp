#pragma once

#include "internals/domapi.hpp"
#include "internals/domkeys.hpp"
#include "internals/jsapi.hpp"
#include "internals/wire.hpp"

#include <wasm-dom/vnode.hpp>
#include <wasm-dom/vnodedata.hpp>

#include <ranges>
#include <unordered_map>

namespace wasmdom::internals
{
    inline void diffAttrs(const VNodeData& oldVnode, const VNodeData& vnode)
    {
        const Attrs& oldAttrs{ oldVnode.attrs };
        const Attrs& attrs{ vnode.attrs };

        for (const auto& [key, _] : oldAttrs) {
            if (!attrs.contains(key)) {
                domapi::removeAttribute(vnode.node, key);
            }
        }

        for (const auto& [key, val] : attrs) {
            const auto oldAttrsIt = oldAttrs.find(key);
            if (oldAttrsIt == oldAttrs.cend() || oldAttrsIt->second != val) {
                domapi::setAttribute(vnode.node, key, val);
            }
        }
    }

    inline void diffProps(const VNodeData& oldVnode, VNodeData& vnode)
    {
        const Props& oldProps{ oldVnode.props };
        const Props& props{ vnode.props };

        const emscripten::val nodeRaws{ emscripten::val::array(
            props | std::views::keys | std::ranges::to<std::vector<std::string>>()
        ) };

        vnode.node.set(nodeRawsKey, nodeRaws);

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                vnode.node.set(key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            const auto oldPropsIt = oldProps.find(key);
            if (oldPropsIt == oldProps.cend() ||
                !val.strictlyEquals(oldPropsIt->second) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(vnode.node[key]))) {
                vnode.node.set(key, val);
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
        const Callbacks& oldCallbacks{ oldVnode.callbacks };
        const Callbacks& callbacks{ vnode.callbacks };

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            eventKey = formatEventKey(key);
            jsapi::removeEventListener_(vnode.node.as_handle(), eventKey.c_str(), vnode.node[nodeEventsKey][eventKey].as_handle());
            vnode.node[nodeEventsKey].delete_(eventKey);
        }

        if (vnode.node[nodeEventsKey].isUndefined()) {
            vnode.node.set(nodeEventsKey, emscripten::val::object());
        }

        for (auto& [key, val] : callbacks) {
            eventKey = formatEventKey(key);
            const emscripten::val jsCallback = toJsCallback(val);
            const emscripten::val functorAdapter = jsCallback["opcall"].call<emscripten::val>("bind", jsCallback);
            jsapi::addEventListener_(vnode.node.as_handle(), eventKey.c_str(), functorAdapter.as_handle());
            vnode.node[nodeEventsKey].set(eventKey, functorAdapter);
        }
    }

    inline void diff(const VNodeData& oldVnode, VNodeData& vnode)
    {
        const std::size_t vnodes{ vnode.hash | oldVnode.hash };

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
