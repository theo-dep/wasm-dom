#include "vdom.hpp"

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
            vnode.setElm(EM_ASM_INT(
                { return
                      // clang-format off
                      $1 === 0
                               // clang-format on
                               ? Module.createElement(
                                     Module['UTF8ToString']($0))
                               : Module.createElementNS(
                                     Module['UTF8ToString']($1),
                                     Module['UTF8ToString']($0)); }, vnode.sel().c_str(), vnode.hash() & hasNS ? vnode.ns().c_str() : 0));
        } else if (vnode.hash() & isText) {
            vnode.setElm(EM_ASM_INT({ return Module.createTextNode(
                                          Module['UTF8ToString']($0)); }, vnode.sel().c_str()));
            return vnode.elm();
        } else if (vnode.hash() & isFragment) {
            vnode.setElm(EM_ASM_INT({
                return Module.createDocumentFragment();
            }));
        } else if (vnode.hash() & isComment) {
            vnode.setElm(EM_ASM_INT({ return Module.createComment(
                                          Module['UTF8ToString']($0)); }, vnode.sel().c_str()));
            return vnode.elm();
        }

        for (VNode& child : vnode._data->children) {
            int elm = createElm(child);
            EM_ASM_({ Module.appendChild($0, $1); }, vnode.elm(), elm);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);

        return vnode.elm();
    }

    void addVNodes(int parentElm,
                   int before,
                   Children& vnodes,
                   Children::size_type startIdx,
                   Children::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            int elm = createElm(vnodes[startIdx++]);
            EM_ASM_({ Module.insertBefore($0, $1, $2) }, parentElm, elm, before);
        }
    }

    void removeVNodes(const Children& vnodes,
                      Children::size_type startIdx,
                      Children::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            const VNode& vnode = vnodes[startIdx++];

            if (vnode) {
                EM_ASM_({ Module.removeChild($0); }, vnode.elm());

                if (vnode.hash() & hasRef) {
                    vnode.callbacks().at("ref")(emscripten::val::null());
                }
            }
        }
    }

    VNode boundCheckVNode(Children& ch, int idx, int endIdx)
    {
        return (idx >= 0 && idx <= endIdx ? ch[idx] : nullptr);
    }

    void updateChildren(int parentElm,
                        Children& oldCh,
                        Children& newCh)
    {
        int oldStartIdx = 0;
        int newStartIdx = 0;
        int oldEndIdx = oldCh.size() - 1;
        int newEndIdx = newCh.size() - 1;
        const int& oldMaxIdx = oldEndIdx; // to avoid unsequenced modification and access
        const int& newMaxIdx = newEndIdx; // |
        VNode oldStartVnode = oldCh[0];
        VNode oldEndVnode = oldCh[oldEndIdx];
        VNode newStartVnode = newCh[0];
        VNode newEndVnode = newCh[newEndIdx];
        bool oldKeys = false;
        std::unordered_map<std::string, int> oldKeyToIdx;

        while (oldStartIdx <= oldEndIdx & newStartIdx <= newEndIdx) {
            if (!oldStartVnode) {
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
            } else if (!oldEndVnode) {
                oldEndVnode = boundCheckVNode(oldCh, --oldEndIdx, oldMaxIdx);
            } else if (sameVNode(oldStartVnode, newStartVnode)) {
                if (oldStartVnode != newStartVnode)
                    patchVNode(oldStartVnode, newStartVnode, parentElm);
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
                newStartVnode = boundCheckVNode(newCh, ++newStartIdx, newMaxIdx);
            } else if (sameVNode(oldEndVnode, newEndVnode)) {
                if (oldEndVnode != newEndVnode)
                    patchVNode(oldEndVnode, newEndVnode, parentElm);
                oldEndVnode = boundCheckVNode(oldCh, --oldEndIdx, oldMaxIdx);
                newEndVnode = boundCheckVNode(newCh, --newEndIdx, newMaxIdx);
            } else if (sameVNode(oldStartVnode, newEndVnode)) {
                if (oldStartVnode != newEndVnode)
                    patchVNode(oldStartVnode, newEndVnode, parentElm);

                EM_ASM_({ Module.insertBefore(
                              $0,
                              $1,
                              Module.nextSibling($2)); }, parentElm, oldStartVnode.elm(), oldEndVnode.elm());
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
                newEndVnode = boundCheckVNode(newCh, --newEndIdx, newMaxIdx);
            } else if (sameVNode(oldEndVnode, newStartVnode)) {
                if (oldEndVnode != newStartVnode)
                    patchVNode(oldEndVnode, newStartVnode, parentElm);

                EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, oldEndVnode.elm(), oldStartVnode.elm());
                oldEndVnode = boundCheckVNode(oldCh, --oldEndIdx, oldMaxIdx);
                newStartVnode = boundCheckVNode(newCh, ++newStartIdx, newMaxIdx);
            } else {
                if (!oldKeys) {
                    oldKeys = true;
                    int beginIdx = oldStartIdx;
                    while (beginIdx <= oldEndIdx) {
                        if (oldCh[beginIdx].hash() & hasKey) {
                            oldKeyToIdx.emplace(oldCh[beginIdx].key(), beginIdx);
                        }
                        ++beginIdx;
                    }
                }
                if (!oldKeyToIdx.contains(newStartVnode.key())) {
                    int elm = createElm(newStartVnode);
                    EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode.elm());
                } else {
                    VNode elmToMove = oldCh[oldKeyToIdx[newStartVnode.key()]];
                    if ((elmToMove.hash() & extractSel) != (newStartVnode.hash() & extractSel)) {
                        int elm = createElm(newStartVnode);
                        EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode.elm());
                    } else {
                        if (elmToMove != newStartVnode)
                            patchVNode(elmToMove, newStartVnode, parentElm);
                        oldCh[oldKeyToIdx[newStartVnode.key()]] = nullptr;
                        EM_ASM_({ Module.insertBefore($0, $1, $2); }, parentElm, elmToMove.elm(), oldStartVnode.elm());
                    }
                }
                newStartVnode = boundCheckVNode(newCh, ++newStartIdx, newMaxIdx);
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
        EM_ASM_({ Module.setNodeValue(
                      $0,
                      Module['UTF8ToString']($1)); }, vnode.elm(), vnode.sel().c_str());
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
        EM_ASM_({
				var parent = Module.parentNode($1);
				if (parent !== 0) {
					Module.insertBefore(
						parent,
						$0,
						Module.nextSibling($1)
					);
					Module.removeChild($1);
				} }, elm, _currentNode.elm());
    }

    _currentNode = newNode;

    return _currentNode;
}
