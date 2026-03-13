#pragma once

#include <wasm-dom/vnode.hpp>
#include <wasm-dom/vnodedata.hpp>

#include <ranges>

namespace wasmdom::internals
{
    inline void updateVNode(const VNodeData& vnodeData, VNode& vnode)
    {
        vnode.setNode(vnodeData.node);
        for (const auto& [vnodeDataChild, vnodeChild] : std::views::zip(vnodeData.children, vnode.children())) {
            updateVNode(*vnodeDataChild, vnodeChild);
        }
    }
}
