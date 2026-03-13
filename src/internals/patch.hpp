#pragma once

#include "internals/deletevnodedata.hpp"
#include "internals/diff.hpp"
#include "internals/domapi.hpp"

#include <wasm-dom/vnode.hpp>
#include <wasm-dom/vnodedata.hpp>

#include <algorithm>

namespace wasmdom::internals
{
    void patchVNode(VNodeData& oldVnode, VNodeData& vnode);

    inline void onEvent(const VNodeData& vnode, const Event& event)
    {
        const EventCallbacks& eventCallbacks{ vnode.eventCallbacks };
        const auto callbackIt = eventCallbacks.find(event);
        if (callbackIt != eventCallbacks.cend()) {
            callbackIt->second(vnode.node);
        }
    }

    inline bool sameVNode(const VNodeData& vnode1, const VNodeData& vnode2)
    {
        return
            // compare selector, nodeType and key existence
            ((vnode1.hash & id) == (vnode2.hash & id)) &&
            // compare keys
            (!(vnode1.hash & hasKey) || (vnode1.key == vnode2.key));
    }

    inline emscripten::val domNode(const VNodeData& vnode)
    {
        if (vnode.hash & isFragment && vnode.parent) {
            // a fragment is not added to the DOM, get parent
            return domNode(*vnode.parent);
        } else {
            return vnode.node;
        }
    }

    inline emscripten::val domSiblingNode(const VNodeData& vnode)
    {
        if (vnode.hash & isFragment) {
            if (vnode.hash & hasChildren) {
                // a fragment is not added to the DOM, get first child
                return domSiblingNode(*vnode.children.front());
            } else {
                return emscripten::val::null();
            }
        } else {
            return vnode.node;
        }
    }

    inline const VNodeData* nextSibling(const VNodeData& vnode)
    {
        // Get vnode position in parent children
        if (!vnode.parent) {
            return nullptr;
        }

        const std::list<VNodeData*>::const_iterator vnodeIt{
            std::ranges::find(vnode.parent->children, &vnode)
        };
        if (vnodeIt == vnode.parent->children.end()) {
            return nullptr;
        }

        const std::list<VNodeData*>::const_iterator nextSiblingVNodeIt{ std::next(vnodeIt) };
        if (nextSiblingVNodeIt == vnode.parent->children.end()) {
            return nullptr;
        }

        return *nextSiblingVNodeIt;
    }

    inline void removeNode(VNodeData& parentVnode, VNodeData* vnode)
    {
        // remove and delete vnode from parent children
        std::erase(parentVnode.children, vnode);
        deleteVNodeData(vnode);
    }

    inline void removeNode(VNodeData& vnode)
    {
        if (vnode.hash & isFragment) {
            // a fragment is not added to the DOM, remove its children
            for (VNodeData* child : vnode.children) {
                removeNode(*child);
            }
        } else {
            if (vnode.parent) {
                removeNode(*vnode.parent, &vnode);
                domapi::removeNode(domNode(*vnode.parent), vnode.node);
            }
        }
    }

    inline void createNode(VNodeData& vnode)
    {
        if (vnode.hash & isElement) {
            if (vnode.hash & hasNS) {
                vnode.node = domapi::createElementNS(vnode.ns, vnode.sel);
            } else {
                vnode.node = domapi::createElement(vnode.sel);
            }
        } else if (vnode.hash & isText) {
            vnode.node = domapi::createTextNode(vnode.sel);
            return;
        } else if (vnode.hash & isFragment) {
            vnode.node = domapi::createDocumentFragment();
        } else if (vnode.hash & isComment) {
            vnode.node = domapi::createComment(vnode.sel);
            return;
        }

        for (VNodeData* child : vnode.children) {
            createNode(*child);
            domapi::appendChild(vnode.node, child->node);
            onEvent(*child, onMount);
        }

        static const VNodeData emptyNode;
        internals::diff(emptyNode, vnode);
    }

    inline void insertBefore(VNodeData& parentVnode, VNodeData& vnode, const VNodeData* referenceVnode)
    {
        const std::list<VNodeData*>::const_iterator vnodeIt{
            std::ranges::find(parentVnode.children, &vnode)
        };
        if (vnodeIt != parentVnode.children.end()) {
            parentVnode.children.erase(vnodeIt);
        }

        const std::list<VNodeData*>::const_iterator referenceChildIt{
            std::ranges::find(parentVnode.children, referenceVnode)
        };
        vnode.parent = &parentVnode;
        parentVnode.children.insert(referenceChildIt, &vnode);
    }

    inline void insertBefore(VNodeData* parentVnode, VNodeData& vnode, const VNodeData* referenceVnode)
    {
        if (parentVnode) {
            insertBefore(*parentVnode, vnode, referenceVnode);
            const emscripten::val refNode{ referenceVnode ? domSiblingNode(*referenceVnode) : emscripten::val::null() };
            domapi::insertBefore(domNode(*parentVnode), vnode.node, refNode);
        }
    }

    inline void addVNodes(
        VNodeData* parentVnode, const VNodeData* referenceVnode,
        std::list<VNodeData*>::iterator start, std::list<VNodeData*>::iterator end
    )
    {
        for (; start != end; ++start) {
            createNode(**start);
            insertBefore(parentVnode, **start, referenceVnode);
            onEvent(**start, onMount);
        }
    }

    inline void unmountVNodeChildren(const VNodeData& vnode)
    {
        for (const VNodeData* child : vnode.children) {
            unmountVNodeChildren(*child);
            onEvent(*child, onUnmount);
        }
    }

    inline void removeVNodes(std::list<VNodeData*>::iterator start, std::list<VNodeData*>::iterator end)
    {
        for (; start != end; ++start) {
            if (*start) {
                unmountVNodeChildren(**start);
                onEvent(**start, onUnmount);
                removeNode(**start);
            }
        }
    }

    inline void updateChildren(
        VNodeData* parentVnode, std::list<VNodeData*>::iterator oldStart, std::list<VNodeData*>::iterator oldEnd,
        std::list<VNodeData*>::iterator newStart, std::list<VNodeData*>::iterator newEnd, std::list<VNodeData*>::iterator end
    )
    {
        bool oldKeys = false;
        std::unordered_map<std::string, std::list<VNodeData*>::iterator> oldKeyTo;

        while (oldStart != std::next(oldEnd) && newStart != std::next(newEnd)) {
            if (!*oldStart) {
                ++oldStart;
            } else if (!*oldEnd) {
                --oldEnd;
            } else if (sameVNode(**oldStart, **newStart)) {
                if (*oldStart != *newStart)
                    patchVNode(**oldStart, **newStart);
                ++oldStart;
                ++newStart;
            } else if (sameVNode(**oldEnd, **newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(**oldEnd, **newEnd);
                --oldEnd;
                --newEnd;
            } else if (sameVNode(**oldStart, **newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(**oldStart, **newEnd);
                insertBefore(parentVnode, **newEnd, nextSibling(**oldEnd));
                ++oldStart;
                --newEnd;
            } else if (sameVNode(**oldEnd, **newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(**oldEnd, **newStart);
                insertBefore(parentVnode, **newStart, *oldStart);
                --oldEnd;
                ++newStart;
            } else {
                if (!oldKeys) {
                    oldKeys = true;

                    for (std::list<VNodeData*>::iterator begin{ oldStart }; begin != std::next(oldEnd); ++begin) {
                        if ((*begin)->hash & hasKey) {
                            oldKeyTo.emplace((*begin)->key, begin);
                        }
                    }
                }
                if (!oldKeyTo.contains((*newStart)->key)) {
                    createNode(**newStart);
                    insertBefore(parentVnode, **newStart, *oldStart);
                    onEvent(**newStart, onMount);
                } else {
                    const std::list<VNodeData*>::iterator elmToMove{ oldKeyTo[(*newStart)->key] };

                    if (((*elmToMove)->hash & extractSel) != ((*newStart)->hash & extractSel)) {
                        createNode(**newStart);
                        insertBefore(parentVnode, **newStart, *oldStart);
                        onEvent(**newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart) {
                            patchVNode(**elmToMove, **newStart);
                            insertBefore(parentVnode, **newStart, *oldStart);
                            onEvent(**newStart, onMount);
                        }
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }

        const std::list<VNodeData*>::iterator newEndEnd{ std::next(newEnd) };
        if (newStart != newEndEnd) {
            const std::list<VNodeData*>::const_iterator beforeVnode{ newEndEnd };
            if (beforeVnode != end) {
                addVNodes(parentVnode, *beforeVnode, newStart, newEndEnd);
            } else {
                addVNodes(parentVnode, nullptr, newStart, newEndEnd);
            }
        }

        const std::list<VNodeData*>::iterator oldEndEnd{ std::next(oldEnd) };
        if (oldStart != oldEndEnd) {
            removeVNodes(oldStart, oldEndEnd);
        }
    }

    inline void patchVNode(VNodeData& oldVnode, VNodeData& vnode)
    {
        if (sameVNode(oldVnode, vnode)) {
            vnode.node = oldVnode.node;
            vnode.parent = oldVnode.parent;

            if (vnode.hash & isElementOrFragment) {
                const std::size_t childrenNotEmpty{ vnode.hash & hasChildren };
                const std::size_t oldChildrenNotEmpty{ oldVnode.hash & hasChildren };

                if (childrenNotEmpty && oldChildrenNotEmpty) {
                    updateChildren(
                        &oldVnode, oldVnode.children.begin(), std::prev(oldVnode.children.end()),
                        vnode.children.begin(), std::prev(vnode.children.end()), vnode.children.end()
                    );
                } else if (childrenNotEmpty) {
                    addVNodes(&oldVnode, nullptr, vnode.children.begin(), vnode.children.end());
                } else if (oldChildrenNotEmpty) {
                    removeVNodes(oldVnode.children.begin(), oldVnode.children.end());
                }

                internals::diff(oldVnode, vnode);
            } else if (vnode.sel != oldVnode.sel) {
                domapi::setNodeValue(vnode.node, vnode.sel);
            }

            onEvent(vnode, onUpdate);
        } else {
            createNode(vnode);
            insertBefore(oldVnode.parent, vnode, nextSibling(oldVnode));
            onEvent(vnode, onMount);
            unmountVNodeChildren(oldVnode);
            onEvent(oldVnode, onUnmount);
            removeNode(oldVnode);
        }
    }
}
