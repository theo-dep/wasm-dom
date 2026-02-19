#pragma once

#include <wasm-dom/vnode.hpp>

#include <emscripten/val.h>

#include <algorithm>

namespace wasmdom::internals
{
    inline VNode toNormalizedVNode(const emscripten::val& node)
    {
        VNode vnode{ VNode::toVNode(node) };
        vnode.normalize();
        return vnode;
    }

    inline VNode toNormalizedVNode(const VNode& parent, const emscripten::val& node)
    {
        if (!parent) {
            return toNormalizedVNode(node);
        }

        const Children::const_iterator vnodeIt{
            std::ranges::find_if(parent, [&node](const VNode& child) {
                return node.strictlyEquals(child.node());
            })
        };

        if (vnodeIt == parent.end()) {
            return toNormalizedVNode(node);
        }

        return *vnodeIt;
    }
}
