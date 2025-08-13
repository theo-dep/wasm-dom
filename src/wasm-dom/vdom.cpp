#include "vdom.hpp"

#include "domapi.hpp"
#include "vnode.hpp"

#include <emscripten.h>

wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode.normalize();
}

namespace wasmdom
{

    void patchVNode(VNode& oldVnode, VNode& vnode, int parentElm);

    bool sameVNode(const VNode& vnode1, const VNode& vnode2)
    {
        return
            // compare selector, nodeType and key existance
            ((vnode1.hash() & id) == (vnode2.hash() & id)) &&
            // compare keys
            (!(vnode1.hash() & hasKey) || (vnode1.key() == vnode2.key()));
    }

    int createElm(VNode& vnode)
    {
        if (vnode.hash() & isElement) {
            if (vnode.hash() & hasNS) {
                vnode.setElm(domapi::createElementNS(vnode.ns(), vnode.sel()));
            } else {
                vnode.setElm(domapi::createElement(vnode.sel()));
            }
        } else if (vnode.hash() & isText) {
            vnode.setElm(domapi::createTextNode(vnode.sel()));
            return vnode.elm();
        } else if (vnode.hash() & isFragment) {
            vnode.setElm(domapi::createDocumentFragment());
        } else if (vnode.hash() & isComment) {
            vnode.setElm(domapi::createComment(vnode.sel()));
            return vnode.elm();
        }

        for (VNode& child : vnode._data->children) {
            int elm = createElm(child);
            domapi::appendChild(vnode.elm(), elm);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);

        return vnode.elm();
    }

    void addVNodes(int parentElm, int before, Children& vnodes, Children::size_type startIdx, Children::size_type endIdx)
    {
        for (; startIdx <= endIdx; ++startIdx) {
            int elm = createElm(vnodes[startIdx]);
            domapi::insertBefore(parentElm, elm, before);
        }
    }

    void removeVNodes(const Children& vnodes, Children::size_type startIdx, Children::size_type endIdx)
    {
        for (; startIdx <= endIdx; ++startIdx) {
            const VNode& vnode = vnodes[startIdx];

            if (vnode) {
                domapi::removeChild(vnode.elm());

                if (vnode.hash() & hasRef) {
                    vnode.callbacks().at("ref")(emscripten::val::null());
                }
            }
        }
    }

    void updateChildren(int parentElm, Children& oldCh, Children& newCh)
    {
        int oldStartIdx = 0;
        int newStartIdx = 0;
        int oldEndIdx = oldCh.size() - 1;
        int newEndIdx = newCh.size() - 1;
        VNode oldStartVnode = oldCh[0];
        VNode oldEndVnode = oldCh[oldEndIdx];
        VNode newStartVnode = newCh[0];
        VNode newEndVnode = newCh[newEndIdx];
        bool oldKeys = false;
        std::unordered_map<std::string, int> oldKeyToIdx;

        while (oldStartIdx <= oldEndIdx && newStartIdx <= newEndIdx) {
            if (!oldStartVnode) {
                oldStartVnode = ++oldStartIdx <= oldEndIdx ? oldCh[oldStartIdx] : nullptr;
            } else if (!oldEndVnode) {
                oldEndVnode = --oldEndIdx >= oldStartIdx ? oldCh[oldEndIdx] : nullptr;
            } else if (sameVNode(oldStartVnode, newStartVnode)) {
                if (oldStartVnode != newStartVnode)
                    patchVNode(oldStartVnode, newStartVnode, parentElm);
                oldStartVnode = ++oldStartIdx <= oldEndIdx ? oldCh[oldStartIdx] : nullptr;
                newStartVnode = ++newStartIdx <= newEndIdx ? newCh[newStartIdx] : nullptr;
            } else if (sameVNode(oldEndVnode, newEndVnode)) {
                if (oldEndVnode != newEndVnode)
                    patchVNode(oldEndVnode, newEndVnode, parentElm);
                oldEndVnode = --oldEndIdx >= oldStartIdx ? oldCh[oldEndIdx] : nullptr;
                newEndVnode = --newEndIdx >= newStartIdx ? newCh[newEndIdx] : nullptr;
            } else if (sameVNode(oldStartVnode, newEndVnode)) {
                if (oldStartVnode != newEndVnode)
                    patchVNode(oldStartVnode, newEndVnode, parentElm);
                int nextSiblingOldPtr = domapi::nextSibling(oldEndVnode.elm());
                domapi::insertBefore(parentElm, oldStartVnode.elm(), nextSiblingOldPtr);
                oldStartVnode = ++oldStartIdx <= oldEndIdx ? oldCh[oldStartIdx] : nullptr;
                newEndVnode = --newEndIdx >= newStartIdx ? newCh[newEndIdx] : nullptr;
            } else if (sameVNode(oldEndVnode, newStartVnode)) {
                if (oldEndVnode != newStartVnode)
                    patchVNode(oldEndVnode, newStartVnode, parentElm);

                domapi::insertBefore(parentElm, oldEndVnode.elm(), oldStartVnode.elm());
                oldEndVnode = --oldEndIdx >= oldStartIdx ? oldCh[oldEndIdx] : nullptr;
                newStartVnode = ++newStartIdx <= newEndIdx ? newCh[newStartIdx] : nullptr;
            } else {
                if (!oldKeys) {
                    oldKeys = true;

                    for (int beginIdx = oldStartIdx; beginIdx <= oldEndIdx; ++beginIdx) {
                        if (oldCh[beginIdx].hash() & hasKey) {
                            oldKeyToIdx.emplace(oldCh[beginIdx].key(), beginIdx);
                        }
                    }
                }
                if (!oldKeyToIdx.contains(newStartVnode.key())) {
                    int elm = createElm(newStartVnode);
                    domapi::insertBefore(parentElm, elm, oldStartVnode.elm());
                } else {
                    VNode elmToMove = oldCh[oldKeyToIdx[newStartVnode.key()]];
                    if ((elmToMove.hash() & extractSel) != (newStartVnode.hash() & extractSel)) {
                        int elm = createElm(newStartVnode);
                        domapi::insertBefore(parentElm, elm, oldStartVnode.elm());
                    } else {
                        if (elmToMove != newStartVnode)
                            patchVNode(elmToMove, newStartVnode, parentElm);
                        oldCh[oldKeyToIdx[newStartVnode.key()]] = nullptr;
                        domapi::insertBefore(parentElm, elmToMove.elm(), oldStartVnode.elm());
                    }
                }
                newStartVnode = ++newStartIdx <= newEndIdx ? newCh[newStartIdx] : nullptr;
            }
        }
        if (oldStartIdx <= oldEndIdx || newStartIdx <= newEndIdx) {
            if (oldStartIdx > oldEndIdx) {
                addVNodes(parentElm, newEndIdx + 1 <= static_cast<int>(newCh.size()) - 1 ? newCh[newEndIdx + 1].elm() : 0, newCh, newStartIdx, newEndIdx);
            } else {
                removeVNodes(oldCh, oldStartIdx, oldEndIdx);
            }
        }
    }

}

void wasmdom::patchVNode(VNode& oldVnode, VNode& vnode, int parentElm)
{
    vnode.setElm(oldVnode.elm());
    if (vnode.hash() & isElementOrFragment) {
        const unsigned int childrenNotEmpty = vnode.hash() & hasChildren;
        const unsigned int oldChildrenNotEmpty = oldVnode.hash() & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode.hash() & isFragment ? parentElm : vnode.elm(), oldVnode._data->children, vnode._data->children);
        } else if (childrenNotEmpty) {
            addVNodes(vnode.hash() & isFragment ? parentElm : vnode.elm(), 0, vnode._data->children, 0, vnode.children().size() - 1);
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode._data->children, 0, oldVnode.children().size() - 1);
        }
        vnode.diff(oldVnode);
    } else if (vnode.sel() != oldVnode.sel()) {
        domapi::setNodeValue(vnode.elm(), vnode.sel());
    }
}

const wasmdom::VNode& wasmdom::VDom::patch(const VNode& vnode)
{
    if (!vnode || _currentNode == vnode)
        return _currentNode;

    VNode newNode = vnode;
    newNode.normalize();

    if (sameVNode(_currentNode, newNode)) {
        patchVNode(_currentNode, newNode, _currentNode.elm());
    } else {
        int elm = createElm(newNode);
        int parentPtr = domapi::parentNode(_currentNode.elm());
        int nextSiblingElmPtr = domapi::nextSibling(_currentNode.elm());
        domapi::insertBefore(parentPtr, elm, nextSiblingElmPtr);
        domapi::removeChild(_currentNode.elm());
    }

    _currentNode = newNode;

    return _currentNode;
}
