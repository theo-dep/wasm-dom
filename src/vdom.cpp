// #include "internals/patch.hpp"
#include "internals/tovnodedata.hpp"
#include "internals/variant.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vdom.hpp>
#include <wasm-dom/vnode.hpp>

#include <algorithm>

WASMDOM_SH_INLINE
wasmdom::VDom::VDom(const emscripten::val& element)
    : _topParentNode{ internals::toVNodeData(element["parentNode"]) }
    , _currentNode{ _topParentNode ? internals::toVNodeData(element, *_topParentNode) : internals::toVNodeData(element) }
{
}

WASMDOM_SH_INLINE
void wasmdom::VDom::patch(VNode vnode)
{
    if (!vnode.valid()) {
        return;
    }

    vnode.normalize();
    VNodeData data{ internals::toVNodeData(vnode, _topParentNode ? &_topParentNode.value() : nullptr) };

    std::visit(
        internals::overloaded{
            [&data](const std::optional<VNodeData> currentNode) {
                (void)data;
                if (currentNode) {
                    // internals::patchVNode(*currentNode, data);
                }
            },
            [&data](const std::reference_wrapper<VNodeData>& currentNode) {
                // internals::patchVNode(currentNode, data);
                (void)data;
                (void)currentNode;
            } },
        _currentNode
    );

    if (_topParentNode) {
        _currentNode = internals::toVNodeData(vnode, *_topParentNode);
    } else {
        _currentNode = std::optional(data);
    }
}
