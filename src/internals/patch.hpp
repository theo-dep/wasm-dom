#pragma once

#include "internals/deletevnodedata.hpp"
#include "internals/diff.hpp"
#include "internals/domapi.hpp"

#include <wasm-dom/vnodedata.hpp>

#include <algorithm>

namespace wasmdom::internals
{
    void patchVNode(VNodeData*& currentVnode, const VNodeData* const newVnode);

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

    inline void removeNode(VNodeData& vnode)
    {
        if (vnode.parent) {
            const emscripten::val parentNode{ domNode(*vnode.parent) };
            if (vnode.hash & isFragment) {
                // a fragment is not added to the DOM, remove its children
                for (VNodeData* const child : vnode.children) {
                    domapi::removeNode(parentNode, child->node);
                }
            } else {
                domapi::removeNode(parentNode, vnode.node);
            }

            std::erase(vnode.parent->children, &vnode);
        }

        deleteVNodeData(&vnode);
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

        for (VNodeData* const child : vnode.children) {
            createNode(*child);
            domapi::appendChild(vnode.node, child->node);
            onEvent(*child, onMount);
        }

        VNodeData emptyNode;
        emptyNode.node = vnode.node;
        internals::diff(emptyNode, vnode);
    }

    inline void insertBefore(VNodeData& parentVnode, VNodeData& vnode, const VNodeData* const referenceVnode)
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

    inline void insertBefore(VNodeData* const parentVnode, VNodeData& vnode, const VNodeData* const referenceVnode)
    {
        if (parentVnode) {
            insertBefore(*parentVnode, vnode, referenceVnode);
            const emscripten::val refNode{ referenceVnode ? domSiblingNode(*referenceVnode) : emscripten::val::null() };
            domapi::insertBefore(domNode(*parentVnode), vnode.node, refNode);
        }
    }

    inline void addVNodes(
        VNodeData* const parentVnode, const VNodeData* const referenceVnode,
        std::list<VNodeData*>::const_iterator start, std::list<VNodeData*>::const_iterator end
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
        for (const VNodeData* const child : vnode.children) {
            unmountVNodeChildren(*child);
            onEvent(*child, onUnmount);
        }
    }

    inline void removeVNodes(std::list<VNodeData*>::const_iterator start, std::list<VNodeData*>::const_iterator end)
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
        VNodeData* const parentVnode, std::list<VNodeData*>::iterator currentStart, std::list<VNodeData*>::iterator currentEnd,
        std::list<VNodeData*>::const_iterator newStart, std::list<VNodeData*>::const_iterator newEnd, std::list<VNodeData*>::const_iterator end
    )
    {
        bool currentKeys = false;
        std::unordered_map<std::string, std::list<VNodeData*>::iterator> currentKeyTo;

        while (currentStart != std::next(currentEnd) && newStart != std::next(newEnd)) {
            if (!*currentStart) {
                ++currentStart;
            } else if (!*currentEnd) {
                --currentEnd;
            } else if (sameVNode(**currentStart, **newStart)) {
                if (*currentStart != *newStart)
                    patchVNode(*currentStart, *newStart);
                ++currentStart;
                ++newStart;
            } else if (sameVNode(**currentEnd, **newEnd)) {
                if (*currentEnd != *newEnd)
                    patchVNode(*currentEnd, *newEnd);
                --currentEnd;
                --newEnd;
            } else if (sameVNode(**currentStart, **newEnd)) {
                if (*currentStart != *newEnd)
                    patchVNode(*currentStart, *newEnd);
                insertBefore(parentVnode, **newEnd, nextSibling(**currentEnd));
                ++currentStart;
                --newEnd;
            } else if (sameVNode(**currentEnd, **newStart)) {
                if (*currentEnd != *newStart)
                    patchVNode(*currentEnd, *newStart);
                insertBefore(parentVnode, **newStart, *currentStart);
                --currentEnd;
                ++newStart;
            } else {
                if (!currentKeys) {
                    currentKeys = true;

                    for (std::list<VNodeData*>::iterator begin{ currentStart }; begin != std::next(currentEnd); ++begin) {
                        if ((*begin)->hash & hasKey) {
                            currentKeyTo.emplace((*begin)->key, begin);
                        }
                    }
                }
                if (!currentKeyTo.contains((*newStart)->key)) {
                    createNode(**newStart);
                    insertBefore(parentVnode, **newStart, *currentStart);
                    onEvent(**newStart, onMount);
                } else {
                    const std::list<VNodeData*>::iterator elmToMove{ currentKeyTo[(*newStart)->key] };

                    if (((*elmToMove)->hash & extractSel) != ((*newStart)->hash & extractSel)) {
                        createNode(**newStart);
                        insertBefore(parentVnode, **newStart, *currentStart);
                        onEvent(**newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart) {
                            patchVNode(*elmToMove, *newStart);
                            insertBefore(parentVnode, **newStart, *currentStart);
                            onEvent(**newStart, onMount);
                        }
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }

        const std::list<VNodeData*>::const_iterator newEndEnd{ std::next(newEnd) };
        if (newStart != newEndEnd) {
            const std::list<VNodeData*>::const_iterator beforeVnode{ newEndEnd };
            if (beforeVnode != end) {
                addVNodes(parentVnode, *beforeVnode, newStart, newEndEnd);
            } else {
                addVNodes(parentVnode, nullptr, newStart, newEndEnd);
            }
        }

        const std::list<VNodeData*>::iterator currentEndEnd{ std::next(currentEnd) };
        if (currentStart != currentEndEnd) {
            removeVNodes(currentStart, currentEndEnd);
        }
    }

    inline void patchVNode(VNodeData*& currentVnode, const VNodeData* const newVnode)
    {
        if (sameVNode(*currentVnode, *newVnode)) {
            if (newVnode->hash & isElementOrFragment) {
                const std::size_t newChildrenNotEmpty{ newVnode->hash & hasChildren };
                const std::size_t currentChildrenNotEmpty{ currentVnode->hash & hasChildren };

                if (newChildrenNotEmpty && currentChildrenNotEmpty) {
                    updateChildren(
                        currentVnode, currentVnode->children.begin(), std::prev(currentVnode->children.end()),
                        newVnode->children.cbegin(), std::prev(newVnode->children.cend()), newVnode->children.cend()
                    );
                } else if (newChildrenNotEmpty) {
                    addVNodes(currentVnode, nullptr, newVnode->children.cbegin(), newVnode->children.cend());
                } else if (currentChildrenNotEmpty) {
                    removeVNodes(currentVnode->children.begin(), currentVnode->children.end());
                }

                internals::diff(*currentVnode, *newVnode);
            } else if (newVnode->sel != currentVnode->sel) {
                domapi::setNodeValue(currentVnode->node, newVnode->sel);
            }

            currentVnode->sel = newVnode->sel;
            currentVnode->key = newVnode->key;
            currentVnode->ns = newVnode->ns;
            currentVnode->hash = newVnode->hash;
            currentVnode->attrs = newVnode->attrs;
            currentVnode->props = newVnode->props;
            currentVnode->callbacks = newVnode->callbacks;
            currentVnode->eventCallbacks = newVnode->eventCallbacks;
            delete newVnode;

            onEvent(*currentVnode, onUpdate);
        } else {
            VNodeData* const parentVnode{ currentVnode->parent };
            const VNodeData* const nextSiblingVnode{ nextSibling(*currentVnode) };

            unmountVNodeChildren(*currentVnode);
            onEvent(*currentVnode, onUnmount);
            removeNode(*currentVnode);

            currentVnode = const_cast<VNodeData*>(newVnode);
            createNode(*currentVnode);
            insertBefore(parentVnode, *currentVnode, nextSiblingVnode);
            onEvent(*currentVnode, onMount);
        }
    }
}
