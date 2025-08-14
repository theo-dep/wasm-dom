#include "vdom.hpp"

#include "domapi.hpp"
#include "vnode.hpp"

#include <emscripten.h>

#include <algorithm>

wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode.normalize();
}

namespace wasmdom
{

    void patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode);

    bool sameVNode(const VNode& vnode1, const VNode& vnode2)
    {
        return
            // compare selector, nodeType and key existence
            ((vnode1.hash() & id) == (vnode2.hash() & id)) &&
            // compare keys
            (!(vnode1.hash() & hasKey) || (vnode1.key() == vnode2.key()));
    }

    void createNode(VNode& vnode)
    {
        if (vnode.hash() & isElement) {
            if (vnode.hash() & hasNS) {
                vnode.setNode(domapi::createElementNS(vnode.ns(), vnode.sel()));
            } else {
                vnode.setNode(domapi::createElement(vnode.sel()));
            }
        } else if (vnode.hash() & isText) {
            vnode.setNode(domapi::createTextNode(vnode.sel()));
            return;
        } else if (vnode.hash() & isFragment) {
            vnode.setNode(domapi::createDocumentFragment());
        } else if (vnode.hash() & isComment) {
            vnode.setNode(domapi::createComment(vnode.sel()));
            return;
        }

        for (VNode& child : vnode._data->children) {
            createNode(child);
            domapi::appendChild(vnode.node(), child.node());
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);
    }

    void addVNodes(const emscripten::val& parentNode, const emscripten::val& beforeNode, Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            createNode(*start);
            domapi::insertBefore(parentNode, start->node(), beforeNode);
        }
    }

    void removeVNodes(Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            if (*start) {
                domapi::removeChild(start->node());

                if (start->hash() & hasRef) {
                    start->callbacks().at("ref")(emscripten::val::null());
                }
            }
        }
    }

    void updateChildren(const emscripten::val& parentNode, Children& oldCh, Children& newCh)
    {
        Children::iterator oldStart = oldCh.begin();
        Children::iterator newStart = newCh.begin();
        Children::iterator oldEnd = std::prev(oldCh.end());
        Children::iterator newEnd = std::prev(newCh.end());

        bool oldKeys = false;
        std::unordered_map<std::string, Children::iterator> oldKeyTo;

        while (oldStart <= oldEnd && newStart <= newEnd) {
            if (!*oldStart) {
                ++oldStart;
            } else if (!*oldEnd) {
                --oldEnd;
            } else if (sameVNode(*oldStart, *newStart)) {
                if (*oldStart != *newStart)
                    patchVNode(*oldStart, *newStart, parentNode);
                ++oldStart;
                ++newStart;
            } else if (sameVNode(*oldEnd, *newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(*oldEnd, *newEnd, parentNode);
                --oldEnd;
                --newEnd;
            } else if (sameVNode(*oldStart, *newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(*oldStart, *newEnd, parentNode);
                const emscripten::val nextSiblingOldPtr = domapi::nextSibling(oldEnd->node());
                domapi::insertBefore(parentNode, oldStart->node(), nextSiblingOldPtr);
                ++oldStart;
                --newEnd;
            } else if (sameVNode(*oldEnd, *newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(*oldEnd, *newStart, parentNode);

                domapi::insertBefore(parentNode, oldEnd->node(), oldStart->node());
                --oldEnd;
                ++newStart;
            } else {
                if (!oldKeys) {
                    oldKeys = true;

                    for (Children::iterator begin = oldStart; begin <= oldEnd; ++begin) {
                        if (begin->hash() & hasKey) {
                            oldKeyTo.emplace(begin->key(), begin);
                        }
                    }
                }
                if (!oldKeyTo.contains(newStart->key())) {
                    createNode(*newStart);
                    domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                } else {
                    const Children::iterator elmToMove = oldKeyTo[newStart->key()];
                    if ((elmToMove->hash() & extractSel) != (newStart->hash() & extractSel)) {
                        createNode(*newStart);
                        domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                    } else {
                        if (*elmToMove != *newStart)
                            patchVNode(*elmToMove, *newStart, parentNode);
                        domapi::insertBefore(parentNode, elmToMove->node(), oldStart->node());
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }
        if (oldStart <= oldEnd || newStart <= newEnd) {
            if (oldStart > oldEnd) {
                addVNodes(parentNode, std::next(newEnd) != newCh.end() ? std::next(newEnd)->node() : emscripten::val::null(), newStart, newEnd);
            } else {
                removeVNodes(oldStart, oldEnd);
            }
        }
    }

}

void wasmdom::patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode)
{
    vnode.setNode(oldVnode.node());
    if (vnode.hash() & isElementOrFragment) {
        const std::size_t childrenNotEmpty = vnode.hash() & hasChildren;
        const std::size_t oldChildrenNotEmpty = oldVnode.hash() & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode.hash() & isFragment ? parentNode : vnode.node(), oldVnode._data->children, vnode._data->children);
        } else if (childrenNotEmpty) {
            addVNodes(vnode.hash() & isFragment ? parentNode : vnode.node(), emscripten::val::null(), vnode._data->children.begin(), std::prev(vnode._data->children.end()));
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode._data->children.begin(), std::prev(oldVnode._data->children.end()));
        }
        vnode.diff(oldVnode);
    } else if (vnode.sel() != oldVnode.sel()) {
        domapi::setNodeValue(vnode.node(), vnode.sel());
    }
}

const wasmdom::VNode& wasmdom::VDom::patch(const VNode& vnode)
{
    if (!vnode || _currentNode == vnode)
        return _currentNode;

    VNode newVnode = vnode;
    newVnode.normalize();

    if (sameVNode(_currentNode, newVnode)) {
        patchVNode(_currentNode, newVnode, _currentNode.node());
    } else {
        createNode(newVnode);
        const emscripten::val parentNode = domapi::parentNode(_currentNode.node());
        const emscripten::val nextSiblingNode = domapi::nextSibling(_currentNode.node());
        domapi::insertBefore(parentNode, newVnode.node(), nextSiblingNode);
        domapi::removeChild(_currentNode.node());
    }

    _currentNode = newVnode;

    return _currentNode;
}
