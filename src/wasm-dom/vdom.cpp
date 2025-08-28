#include "vdom.hpp"

#include "domapi.hpp"
#include "internals/patch.hpp"
#include "vnode.hpp"

wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode.normalize();
}

const wasmdom::VNode& wasmdom::VDom::patch(VNode vnode)
{
    if (!vnode || _currentNode == vnode)
        return _currentNode;

    vnode.normalize();

    if (internals::sameVNode(_currentNode, vnode)) {
        internals::patchVNode(_currentNode, vnode, _currentNode.node());
        internals::onEvent(vnode, onUpdate);
    } else {
        internals::createNode(vnode);
        const emscripten::val parentNode = domapi::parentNode(_currentNode.node());
        const emscripten::val nextSiblingNode = domapi::nextSibling(_currentNode.node());
        domapi::insertBefore(parentNode, vnode.node(), nextSiblingNode);
        internals::onEvent(vnode, onMount);
        internals::unmountVNodeChildren(_currentNode);
        internals::onEvent(_currentNode, onUnmount);
        domapi::removeChild(_currentNode.node());
    }

    _currentNode = vnode;

    return _currentNode;
}
