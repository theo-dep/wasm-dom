#pragma once

#include <wasm-dom/vnode.hpp>
#include <wasm-dom/vnodedata.hpp>

#include <algorithm>

namespace wasmdom::internals
{
    inline VNodeData* toVNodeData(const VNode& vnode, VNodeData* parent)
    {
        VNodeData* data = new VNodeData;
        data->sel = vnode.sel();
        data->key = vnode.key();
        data->ns = vnode.ns();
        data->hash = vnode.hash();
        data->attrs = vnode.attrs();
        data->props = vnode.props();
        data->callbacks = vnode.callbacks();
        data->eventCallbacks = vnode.eventCallbacks();
        data->node = vnode.node();

        data->parent = parent;

        for (const VNode& child : vnode.children()) {
            data->children.push_back(toVNodeData(child, data));
        }

        return data;
    }

    inline VNodeData* toVNodeData(const emscripten::val& node)
    {
        VNode vnode{ VNode::toVNode(node) };
        if (!vnode.valid()) {
            return nullptr;
        }

        vnode.normalize();

        return toVNodeData(vnode, nullptr);
    }

    inline VNodeData* toVNodeData(const emscripten::val& node, const VNodeData& parent)
    {
        const std::list<VNodeData*>::const_iterator dataIt{
            std::ranges::find_if(parent.children, [&node](const VNodeData* child) {
                return node.strictlyEquals(child->node);
            })
        };

        assert(dataIt != parent.children.end() && "VNode not found in parent's children");
        return *dataIt;
    }

    inline VNodeData* toVNodeData(const VNode& vnode, const VNodeData& parent)
    {
        return toVNodeData(vnode.node(), parent);
    }
}
