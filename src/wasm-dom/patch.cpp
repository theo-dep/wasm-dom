#include "patch.hpp"

#include "config.hpp"
#include "diff.hpp"
#include "h.hpp"
#include "tovnode.hpp"
#include "vnode.hpp"

#include <emscripten.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace wasmdom
{

    void patchVNode(
        VNode* __restrict__ const oldVnode,
        VNode* __restrict__ const vnode,
        const int parentElm);

    VNode* const emptyNode = h("");

    VNode* currentNode = NULL;

}

namespace wasmdom
{

    bool sameVNode(const VNode* __restrict__ const vnode1, const VNode* __restrict__ const vnode2)
    {
        return
            // compare selector, nodeType and key existance
            ((vnode1->hash & id) == (vnode2->hash & id)) &
            // compare keys
            (!(vnode1->hash & hasKey) || vnode1->key == vnode2->key);
    }

    int createElm(VNode* const vnode)
    {
        if (vnode->hash & isElement) {
            vnode->elm = EM_ASM_INT(
                { return
                      // clang-format off
                      $1 === 0
                               // clang-format on
                               ? Module.createElement(
                                     Module['UTF8ToString']($0))
                               : Module.createElementNS(
                                     Module['UTF8ToString']($1),
                                     Module['UTF8ToString']($0)); }, vnode->sel.c_str(), vnode->hash & hasNS ? vnode->ns.c_str() : 0);
        } else if (vnode->hash & isText) {
            return vnode->elm = EM_ASM_INT({ return Module.createTextNode(
                                                 Module['UTF8ToString']($0)); }, vnode->sel.c_str());
        } else if (vnode->hash & isFragment) {
            vnode->elm = EM_ASM_INT({
                return Module.createDocumentFragment();
            });
        } else if (vnode->hash & isComment) {
            return vnode->elm = EM_ASM_INT({ return Module.createComment(
                                                 Module['UTF8ToString']($0)); }, vnode->sel.c_str());
        }

        for (std::vector<VNode*>::size_type i = 0, j = vnode->children.size(); i != j; ++i) {
            int elm = createElm(vnode->children[i]);
            EM_ASM_({ Module.appendChild($0, $1); }, vnode->elm, elm);
        }

        diff(emptyNode, vnode);

        return vnode->elm;
    }

    void addVNodes(
        const int parentElm,
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

    void removeVNodes(
        const std::vector<VNode*>& vnodes,
        std::vector<VNode*>::size_type startIdx,
        const std::vector<VNode*>::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            VNode* const vnode = vnodes[startIdx++];

            if (vnode) {
                EM_ASM_({ Module.removeChild($0); }, vnode->elm);

                if (vnode->hash & hasRef) {
                    vnode->data.callbacks["ref"](
                        emscripten::val::null());
                }
            }
        }
    }

    void updateChildren(
        int parentElm,
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
                              Module.nextSibling($2)); }, parentElm, oldStartVnode->elm, oldEndVnode->elm);
                oldStartVnode = oldCh[++oldStartIdx];
                newEndVnode = newCh[--newEndIdx];
            } else if (sameVNode(oldEndVnode, newStartVnode)) {
                if (oldEndVnode != newStartVnode)
                    patchVNode(oldEndVnode, newStartVnode, parentElm);

                EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, oldEndVnode->elm, oldStartVnode->elm);
                oldEndVnode = oldCh[--oldEndIdx];
                newStartVnode = newCh[++newStartIdx];
            } else {
                if (!oldKeys) {
                    oldKeys = true;
                    int beginIdx = oldStartIdx;
                    while (beginIdx <= oldEndIdx) {
                        if (oldCh[beginIdx]->hash & hasKey) {
                            oldKeyToIdx.insert(std::make_pair(oldCh[beginIdx]->key, beginIdx));
                        }
                        ++beginIdx;
                    }
                }
                if (!oldKeyToIdx.count(newStartVnode->key)) {
                    int elm = createElm(newStartVnode);
                    EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode->elm);
                } else {
                    VNode* elmToMove = oldCh[oldKeyToIdx[newStartVnode->key]];
                    if ((elmToMove->hash & extractSel) != (newStartVnode->hash & extractSel)) {
                        int elm = createElm(newStartVnode);
                        EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode->elm);
                    } else {
                        if (elmToMove != newStartVnode)
                            patchVNode(elmToMove, newStartVnode, parentElm);
                        oldCh[oldKeyToIdx[newStartVnode->key]] = NULL;
                        EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elmToMove->elm, oldStartVnode->elm);
                    }
                }
                newStartVnode = newCh[++newStartIdx];
            }
        }
        if (oldStartIdx <= oldEndIdx | newStartIdx <= newEndIdx) {
            if (oldStartIdx > oldEndIdx) {
                addVNodes(parentElm, newEndIdx + 1 <= newCh.size() - 1 ? newCh[newEndIdx + 1]->elm : 0, newCh, newStartIdx, newEndIdx);
            } else {
                removeVNodes(oldCh, oldStartIdx, oldEndIdx);
            }
        }
    }

}

void wasmdom::patchVNode(VNode* __restrict__ const oldVnode, VNode* __restrict__ const vnode, const int parentElm)
{
    vnode->elm = oldVnode->elm;
    if (vnode->hash & isElementOrFragment) {
        const unsigned int childrenNotEmpty = vnode->hash & hasChildren;
        const unsigned int oldChildrenNotEmpty = oldVnode->hash & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode->hash & isFragment ? parentElm : vnode->elm, oldVnode->children, vnode->children);
        } else if (childrenNotEmpty) {
            addVNodes(vnode->hash & isFragment ? parentElm : vnode->elm, 0, vnode->children, 0, vnode->children.size() - 1);
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode->children, 0, oldVnode->children.size() - 1);
        }
        diff(oldVnode, vnode);
    } else if (vnode->sel != oldVnode->sel) {
        EM_ASM_({ Module.setNodeValue(
                      $0,
                      Module['UTF8ToString']($1)); }, vnode->elm, vnode->sel.c_str());
    }
}

wasmdom::VNode* wasmdom::patch(const emscripten::val& element, VNode* const vnode)
{
    VNode* oldVnode = toVNode(element);
    VNode* result = patch(oldVnode, vnode);
    if (!CLEAR_MEMORY) {
        deleteVNode(oldVnode);
    }
    return result;
}

wasmdom::VNode* wasmdom::patch(VNode* const oldVnode, VNode* const vnode)
{
    if (!UNSAFE_PATCH &&
        currentNode != oldVnode &&
        currentNode)
        return NULL;

    if (oldVnode == vnode)
        return vnode;

    currentNode = vnode;

    oldVnode->normalize();
    vnode->normalize();

    if (sameVNode(oldVnode, vnode)) {
        patchVNode(oldVnode, vnode, oldVnode->elm);
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
				} }, elm, oldVnode->elm);
    }

    if (CLEAR_MEMORY) {
        deleteVNode(oldVnode);
    }

    return vnode;
}
