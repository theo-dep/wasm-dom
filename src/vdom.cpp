#include "internals/deletevnodedata.hpp"
#include "internals/patch.hpp"
#include "internals/tovnodedata.hpp"
#include "internals/updatevnode.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vdom.hpp>
#include <wasm-dom/vnode.hpp>

WASMDOM_SH_INLINE
wasmdom::VDom::VDom(const emscripten::val& element)
    : _topParentNode{ internals::toVNodeData(element["parentNode"]) }
    , _currentNode{ _topParentNode ? internals::toVNodeData(element, *_topParentNode) : internals::toVNodeData(element) }
{
}

WASMDOM_SH_INLINE
wasmdom::VDom::~VDom()
{
    if (_topParentNode) {
        internals::deleteVNodeData(_topParentNode);
    } else {
        internals::deleteVNodeData(_currentNode);
    }
}

WASMDOM_SH_INLINE
wasmdom::VNode wasmdom::VDom::patch(VNode vnode)
{
    if (!_currentNode || !vnode.valid()) {
        return nullptr;
    }

    vnode.normalize();
    VNodeData* const data{ internals::toVNodeData(vnode, _topParentNode) };

    internals::patchVNode(_currentNode, data);

    internals::updateVNode(*_currentNode, vnode);
    return vnode;
}
