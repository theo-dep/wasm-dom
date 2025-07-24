#include "vdom.hpp"

#include "vnode.hpp"

#include <emscripten.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode->normalize();
}

wasmdom::VDom::~VDom()
{
    delete _currentNode;
}

namespace wasmdom
{

    void patchVNode(const VNode* oldVnode, VNode* vnode, int parentElm);

    const VNode emptyNode = VNode("");

}

namespace wasmdom
{

    bool sameVNode(const VNode* vnode1, const VNode* vnode2)
    {
        return
            // compare selector, nodeType and key existance
            ((vnode1->hash() & id) == (vnode2->hash() & id)) &&
            // compare keys
            (!(vnode1->hash() & hasKey) || (vnode1->key() == vnode2->key()));
    }

    int createElm(VNode* vnode)
    {
        if (vnode->hash() & isElement) {
            vnode->setElm(EM_ASM_INT(
                { return
                      // clang-format off
                      $1 === 0
                               // clang-format on
                               ? Module.createElement(
                                     Module['UTF8ToString']($0))
                               : Module.createElementNS(
                                     Module['UTF8ToString']($1),
                                     Module['UTF8ToString']($0)); }, vnode->sel().c_str(), vnode->hash() & hasNS ? vnode->ns().c_str() : 0));
        } else if (vnode->hash() & isText) {
            vnode->setElm(EM_ASM_INT({ return Module.createTextNode(
                                           Module['UTF8ToString']($0)); }, vnode->sel().c_str()));
            return vnode->elm();
        } else if (vnode->hash() & isFragment) {
            vnode->setElm(EM_ASM_INT({
                return Module.createDocumentFragment();
            }));
        } else if (vnode->hash() & isComment) {
            vnode->setElm(EM_ASM_INT({ return Module.createComment(
                                           Module['UTF8ToString']($0)); }, vnode->sel().c_str()));
            return vnode->elm();
        }

        for (VNode* child : vnode->children()) {
            int elm = createElm(child);
            EM_ASM_({ Module.appendChild($0, $1); }, vnode->elm(), elm);
        }

        vnode->diff(&emptyNode);

        return vnode->elm();
    }

    void addVNodes(const int parentElm,
                   const int before,
                   const std::vector<VNode*>& vnodes,
                   std::vector<VNode*>::size_type startIdx,
                   const std::vector<VNode*>::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            int elm = createElm(vnodes[startIdx++]);
            EM_ASM_({ Module.insertBefore($0, $1, $2) }, parentElm, elm, before);
        }
    }

    void removeVNodes(const std::vector<VNode*>& vnodes,
                      std::vector<VNode*>::size_type startIdx,
                      const std::vector<VNode*>::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            VNode* const vnode = vnodes[startIdx++];

            if (vnode) {
                EM_ASM_({ Module.removeChild($0); }, vnode->elm());

                if (vnode->hash() & hasRef) {
                    vnode->callbacks().at("ref")(emscripten::val::null());
                }
            }
        }
    }

    void updateChildren(int parentElm,
                        std::vector<VNode*> oldCh,
                        const std::vector<VNode*>& newCh)
    {
        int oldStartIdx = 0;
        int newStartIdx = 0;
        int oldEndIdx = oldCh.size() - 1;
        int newEndIdx = newCh.size() - 1;
        VNode* oldStartVnode = oldCh[0];
        VNode* oldEndVnode = oldCh[oldEndIdx];
        VNode* newStartVnode = newCh[0];
        VNode* newEndVnode = newCh[newEndIdx];
        bool oldKeys = false;
        std::unordered_map<std::string, int> oldKeyToIdx;

        while (oldStartIdx <= oldEndIdx & newStartIdx <= newEndIdx) {
            if (!oldStartVnode) {
                oldStartVnode = oldCh[++oldStartIdx];
            } else if (!oldEndVnode) {
                oldEndVnode = oldCh[--oldEndIdx];
            } else if (sameVNode(oldStartVnode, newStartVnode)) {
                if (oldStartVnode != newStartVnode)
                    patchVNode(oldStartVnode, newStartVnode, parentElm);
                oldStartVnode = oldCh[++oldStartIdx];
                newStartVnode = newCh[++newStartIdx];
            } else if (sameVNode(oldEndVnode, newEndVnode)) {
                if (oldEndVnode != newEndVnode)
                    patchVNode(oldEndVnode, newEndVnode, parentElm);
                oldEndVnode = oldCh[--oldEndIdx];
                newEndVnode = newCh[--newEndIdx];
            } else if (sameVNode(oldStartVnode, newEndVnode)) {
                if (oldStartVnode != newEndVnode)
                    patchVNode(oldStartVnode, newEndVnode, parentElm);

                EM_ASM_({ Module.insertBefore(
                              $0,
                              $1,
                              Module.nextSibling($2)); }, parentElm, oldStartVnode->elm(), oldEndVnode->elm());
                oldStartVnode = oldCh[++oldStartIdx];
                newEndVnode = newCh[--newEndIdx];
            } else if (sameVNode(oldEndVnode, newStartVnode)) {
                if (oldEndVnode != newStartVnode)
                    patchVNode(oldEndVnode, newStartVnode, parentElm);

                EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, oldEndVnode->elm(), oldStartVnode->elm());
                oldEndVnode = oldCh[--oldEndIdx];
                newStartVnode = newCh[++newStartIdx];
            } else {
                if (!oldKeys) {
                    oldKeys = true;
                    int beginIdx = oldStartIdx;
                    while (beginIdx <= oldEndIdx) {
                        if (oldCh[beginIdx]->hash() & hasKey) {
                            oldKeyToIdx.insert(std::make_pair(oldCh[beginIdx]->key(), beginIdx));
                        }
                        ++beginIdx;
                    }
                }
                if (!oldKeyToIdx.count(newStartVnode->key())) {
                    int elm = createElm(newStartVnode);
                    EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode->elm());
                } else {
                    VNode* elmToMove = oldCh[oldKeyToIdx[newStartVnode->key()]];
                    if ((elmToMove->hash() & extractSel) != (newStartVnode->hash() & extractSel)) {
                        int elm = createElm(newStartVnode);
                        EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode->elm());
                    } else {
                        if (elmToMove != newStartVnode)
                            patchVNode(elmToMove, newStartVnode, parentElm);
                        oldCh[oldKeyToIdx[newStartVnode->key()]] = nullptr;
                        EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elmToMove->elm(), oldStartVnode->elm());
                    }
                }
                newStartVnode = newCh[++newStartIdx];
            }
        }
        if (oldStartIdx <= oldEndIdx | newStartIdx <= newEndIdx) {
            if (oldStartIdx > oldEndIdx) {
                addVNodes(parentElm, newEndIdx + 1 <= static_cast<int>(newCh.size()) - 1 ? newCh[newEndIdx + 1]->elm() : 0, newCh, newStartIdx, newEndIdx);
            } else {
                removeVNodes(oldCh, oldStartIdx, oldEndIdx);
            }
        }
    }

}

void wasmdom::patchVNode(const VNode* oldVnode, VNode* vnode, int parentElm)
{
    vnode->setElm(oldVnode->elm());
    if (vnode->hash() & isElementOrFragment) {
        const unsigned int childrenNotEmpty = vnode->hash() & hasChildren;
        const unsigned int oldChildrenNotEmpty = oldVnode->hash() & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode->hash() & isFragment ? parentElm : vnode->elm(), oldVnode->children(), vnode->children());
        } else if (childrenNotEmpty) {
            addVNodes(vnode->hash() & isFragment ? parentElm : vnode->elm(), 0, vnode->children(), 0, vnode->children().size() - 1);
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode->children(), 0, oldVnode->children().size() - 1);
        }
        vnode->diff(oldVnode);
    } else if (vnode->sel() != oldVnode->sel()) {
        EM_ASM_({ Module.setNodeValue(
                      $0,
                      Module['UTF8ToString']($1)); }, vnode->elm(), vnode->sel().c_str());
    }
}

wasmdom::VNode* wasmdom::VDom::patch(VNode* vnode)
{
    if (_currentNode == vnode)
        return _currentNode;

    vnode->normalize();

    if (sameVNode(_currentNode, vnode)) {
        patchVNode(_currentNode, vnode, _currentNode->elm());
    } else {
        int elm = createElm(vnode);
        EM_ASM_({
				var parent = Module.parentNode($1);
				if (parent !== 0) {
					Module.insertBefore(
						parent,
						$0,
						Module.nextSibling($1)
					);
					Module.removeChild($1);
				} }, elm, _currentNode->elm());
    }

    delete _currentNode;
    _currentNode = vnode;

    return vnode;
}
