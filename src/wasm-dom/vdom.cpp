#include "vdom.hpp"

#include "internals/conf.h"
#include "internals/domapi.hpp"
#include "internals/patch.hpp"
#include "vnode.hpp"

WASMDOM_SH_INLINE
wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode.normalize();
}

WASMDOM_SH_INLINE
const wasmdom::VNode& wasmdom::VDom::patch(VNode vnode)
{
    if (!_currentNode || !vnode || _currentNode == vnode)
        return _currentNode;

    vnode.normalize();

    if (internals::sameVNode(_currentNode, vnode)) {
        internals::patchVNode(_currentNode, vnode, _currentNode.node());
        internals::onEvent(vnode, onUpdate);
    } else {
        internals::createNode(vnode);
        const emscripten::val parentNode = internals::domapi::parentNode(_currentNode.node());
        const emscripten::val nextSiblingNode = internals::domapi::nextSibling(_currentNode.node());
        internals::domapi::insertBefore(parentNode, vnode.node(), nextSiblingNode);
        internals::onEvent(vnode, onMount);
        internals::unmountVNodeChildren(_currentNode);
        internals::onEvent(_currentNode, onUnmount);
        internals::domapi::removeChild(_currentNode.node());
    }

    _currentNode = vnode;

    return _currentNode;
}
