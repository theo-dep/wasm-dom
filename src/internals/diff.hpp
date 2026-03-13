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
    inline void diffAttrs(const VNodeData& currentVnode, const VNodeData& newVnode)
    {
        const Attrs& currentAttrs{ currentVnode.attrs };
        const Attrs& newAttrs{ newVnode.attrs };

        for (const auto& [key, _] : currentAttrs) {
            if (!newAttrs.contains(key)) {
                domapi::removeAttribute(currentVnode.node, key);
            }
        }

        for (const auto& [key, val] : newAttrs) {
            const auto currentAttrsIt = currentAttrs.find(key);
            if (currentAttrsIt == currentAttrs.cend() || currentAttrsIt->second != val) {
                domapi::setAttribute(currentVnode.node, key, val);
            }
        }
    }

    inline void diffProps(VNodeData& currentVnode, const VNodeData& newVnode)
    {
        const Props& currentProps{ currentVnode.props };
        const Props& newProps{ newVnode.props };

        const emscripten::val nodeRaws{ emscripten::val::array(
            newProps | std::views::keys | std::ranges::to<std::vector<std::string>>()
        ) };

        currentVnode.node.set(nodeRawsKey, nodeRaws);

        for (const auto& [key, _] : currentProps) {
            if (!newProps.contains(key)) {
                currentVnode.node.set(key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : newProps) {
            const auto currentPropsIt = currentProps.find(key);
            if (currentPropsIt == currentProps.cend() ||
                !val.strictlyEquals(currentPropsIt->second) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(currentVnode.node[key]))) {
                currentVnode.node.set(key, val);
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

    inline void diffCallbacks(VNodeData& currentVnode, const VNodeData& newVnode)
    {
        const Callbacks& currentCallbacks{ currentVnode.callbacks };
        const Callbacks& newCallbacks{ newVnode.callbacks };

        std::string eventKey;

        for (const auto& [key, _] : currentCallbacks) {
            eventKey = formatEventKey(key);
            jsapi::removeEventListener_(currentVnode.node.as_handle(), eventKey.c_str(), currentVnode.node[nodeEventsKey][eventKey].as_handle());
            currentVnode.node[nodeEventsKey].delete_(eventKey);
        }

        if (currentVnode.node[nodeEventsKey].isUndefined()) {
            currentVnode.node.set(nodeEventsKey, emscripten::val::object());
        }

        for (auto& [key, val] : newCallbacks) {
            eventKey = formatEventKey(key);
            const emscripten::val jsCallback = toJsCallback(val);
            const emscripten::val functorAdapter = jsCallback["opcall"].call<emscripten::val>("bind", jsCallback);
            jsapi::addEventListener_(currentVnode.node.as_handle(), eventKey.c_str(), functorAdapter.as_handle());
            currentVnode.node[nodeEventsKey].set(eventKey, functorAdapter);
        }
    }

    inline void diff(VNodeData& currentVnode, const VNodeData& newVnode)
    {
        const std::size_t vnodes{ newVnode.hash | currentVnode.hash };

        if (vnodes & hasAttrs) {
            diffAttrs(currentVnode, newVnode);
        }
        if (vnodes & hasProps) {
            diffProps(currentVnode, newVnode);
        }
        if (vnodes & hasCallbacks) {
            diffCallbacks(currentVnode, newVnode);
        }
    }
}
