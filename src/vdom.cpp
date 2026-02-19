#include "internals/patch.hpp"
#include "internals/tovnode.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vdom.hpp>
#include <wasm-dom/vnode.hpp>

WASMDOM_SH_INLINE
wasmdom::VDom::VDom(const emscripten::val& element)
    : _topParentNode{ internals::toNormalizedVNode(element["parentNode"]) }
    , _currentNode{ internals::toNormalizedVNode(_topParentNode, element) }
{
}

WASMDOM_SH_INLINE
const wasmdom::VNode& wasmdom::VDom::patch(VNode vnode)
{
    if (!_currentNode || !vnode || _currentNode == vnode)
        return _currentNode;

    vnode.updateParent(_currentNode);
    vnode.normalize();

    internals::patchVNode(_currentNode, vnode);
    _currentNode = vnode;

    return _currentNode;
}
