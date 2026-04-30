#pragma once

#include "internals/domapi.hpp"
#include "internals/domkeys.hpp"
#include "internals/handletable.hpp"
#include "internals/wire.hpp"

#include <wasm-dom/attribute.hpp>
#include <wasm-dom/vnode.hpp>

#include <ranges>

namespace wasmdom::internals
{
    inline void diffAttrs(const VNode& oldVnode, const VNode& vnode)
    {
        const Attrs& oldAttrs = oldVnode.attrs();
        const Attrs& attrs = vnode.attrs();

        const NodeId nodeId = vnode.nodeId();

        for (const auto& [key, _] : oldAttrs) {
            if (!attrs.contains(key)) {
                domapi::removeAttribute(nodeId, key);
            }
        }

        for (const auto& [key, val] : attrs) {
            const auto oldAttrsIt = oldAttrs.find(key);
            if (oldAttrsIt == oldAttrs.cend() || oldAttrsIt->second != val) {
                domapi::setAttribute(nodeId, key, val);
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

        const NodeId nodeId = vnode.nodeId();
        domapi::setProperty(nodeId, nodeRawsKey, nodeRaws);

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                domapi::setProperty(nodeId, key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            const auto oldPropsIt = oldProps.find(key);
            if (oldPropsIt == oldProps.cend() ||
                !val.strictlyEquals(oldPropsIt->second) ||
                key == "value" || key == "checked") {
                domapi::setProperty(nodeId, key, val);
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

        const NodeId nodeId = vnode.nodeId();
        auto& installed = vnode.installedListeners();

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            eventKey = formatEventKey(key);
            const auto installedIt = installed.find(eventKey);
            if (installedIt != installed.end()) {
                domapi::removeEventListener(nodeId, eventKey, installedIt->second);
                domapi::deleteEventsProperty(nodeId, eventKey);
                installed.erase(installedIt);
            }
        }

        domapi::ensureEventsObject(nodeId);

        for (auto& [key, val] : callbacks) {
            eventKey = formatEventKey(key);
            const emscripten::val jsCallback = toJsCallback(val);
            const emscripten::val functorAdapter = jsCallback["opcall"].call<emscripten::val>("bind", jsCallback);
            domapi::addEventListener(nodeId, eventKey, functorAdapter);
            domapi::setEventsProperty(nodeId, eventKey, functorAdapter);
            installed[eventKey] = functorAdapter;
        }
    }

}
