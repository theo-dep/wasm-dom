#include "internals/deletevnodedata.hpp"
#include "internals/patch.hpp"
#include "internals/tovnodedata.hpp"

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
void wasmdom::VDom::patch(VNode vnode)
{
    if (!_currentNode || !vnode.valid()) {
        return;
    }

    vnode.normalize();
    VNodeData* data{ internals::toVNodeData(vnode, _topParentNode ? _topParentNode : nullptr) };

    internals::patchVNode(*_currentNode, *data);
}
