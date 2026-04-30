#pragma once

#include "internals/domapi.hpp"
#include "internals/domkeys.hpp"
#include "internals/wire.hpp"

#include <wasm-dom/attribute.hpp>
#include <wasm-dom/vnode.hpp>

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

        const emscripten::val& node = vnode.node();
        domapi::setProperty(node, nodeRawsKey, nodeRaws);

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                domapi::setProperty(node, key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            const auto oldPropsIt = oldProps.find(key);
            if (oldPropsIt == oldProps.cend() ||
                !val.strictlyEquals(oldPropsIt->second) ||
                key == "value" || key == "checked") {
                domapi::setProperty(node, key, val);
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

        const emscripten::val& node = vnode.node();
        auto& installed = vnode.installedListeners();

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            eventKey = formatEventKey(key);
            const auto installedIt = installed.find(eventKey);
            if (installedIt != installed.end()) {
                domapi::removeEventListener(node, eventKey, installedIt->second);
                domapi::deleteEventsProperty(node, eventKey);
                installed.erase(installedIt);
            }
        }

        domapi::ensureEventsObject(node);

        for (auto& [key, val] : callbacks) {
            eventKey = formatEventKey(key);
            const emscripten::val jsCallback = toJsCallback(val);
            const emscripten::val functorAdapter = jsCallback["opcall"].call<emscripten::val>("bind", jsCallback);
            domapi::addEventListener(node, eventKey, functorAdapter);
            domapi::setEventsProperty(node, eventKey, functorAdapter);
            installed[eventKey] = functorAdapter;
        }
    }

}
