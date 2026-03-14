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

    inline void removeNode(const VNodeData& parentVnode, const VNodeData& vnode)
    {
        const emscripten::val parentNode{ domNode(parentVnode) };
        if (vnode.hash & isFragment) {
            // a fragment is not added to the DOM, remove its children
            for (const auto& child : vnode.children) {
                domapi::removeNode(parentNode, child->node);
            }
        } else {
            domapi::removeNode(parentNode, vnode.node);
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

        for (const auto& child : vnode.children) {
            createNode(*child);
            domapi::appendChild(vnode.node, child->node);
            onEvent(*child, onMount);
        }

        VNodeData emptyNode;
        emptyNode.node = vnode.node;
        internals::diff(emptyNode, vnode);
    }

    inline void insertBefore(VNodeData* const parentVnode, VNodeData& vnode, const VNodeData* const referenceVnode)
    {
        vnode.parent = parentVnode;
        if (parentVnode) {
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

        if (parentVnode) {
            const auto referenceChildIt = std::ranges::find(parentVnode->children, referenceVnode);
            parentVnode->children.insert(referenceChildIt, start, end);
        }
    }

    inline void unmountVNodeChildren(const VNodeData& vnode)
    {
        for (const auto& child : vnode.children) {
            unmountVNodeChildren(*child);
            onEvent(*child, onUnmount);
        }
    }

    inline void removeVNodes(VNodeData* const parentVnode, std::list<VNodeData*>::const_iterator start, std::list<VNodeData*>::const_iterator end)
    {
        if (parentVnode) {
            for (; start != end; ++start) {
                if (*start) {
                    unmountVNodeChildren(**start);
                    onEvent(**start, onUnmount);
                    removeNode(*parentVnode, **start);
                    deleteVNodeData(*start);
                }
            }

            parentVnode->children.erase(start, end);
        }
    }

    inline void updateChildren(
        VNodeData* const parentVnode,
        std::list<VNodeData*>& currentChildren, const std::list<VNodeData*>& newChildren,
        std::size_t currentStart, std::size_t currentEnd,
        std::size_t newStart, std::size_t newEnd
    )
    {
        bool currentKeys = false;
        std::unordered_map<std::string, std::size_t> currentKeyTo;

        while (currentStart <= currentEnd && newStart <= newEnd) {
            const auto& currentVnodeStart{ std::next(currentChildren.begin(), currentStart) };
            const auto& currentVnodeEnd{ std::next(currentChildren.begin(), currentEnd) };
            const auto& newVnodeStart{ std::next(newChildren.begin(), newStart) };
            const auto& newVnodeEnd{ std::next(newChildren.begin(), newEnd) };

            if (!*currentVnodeStart) {
                ++currentStart;
            } else if (!*currentVnodeEnd) {
                --currentEnd;
            } else if (sameVNode(**currentVnodeStart, **newVnodeStart)) {
                if (*currentVnodeStart != *newVnodeStart)
                    patchVNode(*currentVnodeStart, *newVnodeStart);
                ++currentStart;
                ++newStart;
            } else if (sameVNode(**currentVnodeEnd, **newVnodeEnd)) {
                if (*currentVnodeEnd != *newVnodeEnd)
                    patchVNode(*currentVnodeEnd, *newVnodeEnd);
                --currentEnd;
                --newEnd;
            } else if (sameVNode(**currentVnodeStart, **newVnodeEnd)) {
                if (*currentVnodeStart != *newVnodeEnd)
                    patchVNode(*currentVnodeStart, *newVnodeEnd);
                const auto nextSiblingVnode = std::next(currentVnodeEnd);
                insertBefore(parentVnode, **currentVnodeStart, *nextSiblingVnode);
                if (parentVnode) {
                    const auto currentVnode = *currentVnodeStart;
                    parentVnode->children.erase(currentVnodeStart);
                    parentVnode->children.insert(nextSiblingVnode, currentVnode);
                }
                ++currentStart;
                --newEnd;
            } else if (sameVNode(**currentVnodeEnd, **newVnodeStart)) {
                if (*currentVnodeEnd != *newVnodeStart)
                    patchVNode(*currentVnodeEnd, *newVnodeStart);
                insertBefore(parentVnode, **currentVnodeEnd, *currentVnodeStart);
                if (parentVnode) {
                    const auto currentVnode = *currentVnodeEnd;
                    parentVnode->children.erase(currentVnodeEnd);
                    parentVnode->children.insert(currentVnodeStart, currentVnode);
                }
                --currentEnd;
                ++newStart;
            } else {
                if (!currentKeys) {
                    currentKeys = true;

                    for (auto begin = currentStart; begin <= currentEnd; ++begin) {
                        const auto currentVnode = std::next(currentVnodeStart, currentStart);
                        if ((*currentVnode)->hash & hasKey) {
                            currentKeyTo.emplace((*currentVnode)->key, begin);
                        }
                    }
                }

                const auto elmToMoveIt{ currentKeyTo.find((*newVnodeStart)->key) };
                if (elmToMoveIt == currentKeyTo.end()) {
                    createNode(**newVnodeStart);
                    insertBefore(parentVnode, **newVnodeStart, *currentVnodeStart);
                    if (parentVnode)
                        parentVnode->children.insert(currentVnodeStart, *newVnodeStart);
                    onEvent(**newVnodeStart, onMount);
                } else {
                    const auto elmToMove = elmToMoveIt->second;
                    const auto currentVnodeIt = std::next(currentVnodeStart, elmToMove);

                    if (((*currentVnodeIt)->hash & extractSel) != ((*newVnodeStart)->hash & extractSel)) {
                        createNode(**newVnodeStart);
                        insertBefore(parentVnode, **newVnodeStart, *currentVnodeStart);
                        if (parentVnode)
                            parentVnode->children.insert(currentVnodeStart, *newVnodeStart);
                        onEvent(**newVnodeStart, onMount);
                    } else {
                        if (*currentVnodeIt != *newVnodeStart) {
                            patchVNode(*currentVnodeIt, *newVnodeStart);
                            insertBefore(parentVnode, **currentVnodeIt, *currentVnodeStart);
                            if (parentVnode) {
                                const auto currentVnode = *currentVnodeIt;
                                parentVnode->children.erase(currentVnodeIt);
                                parentVnode->children.insert(currentVnodeStart, currentVnode);
                            }
                            onEvent(**newVnodeStart, onMount);
                        }
                        currentKeyTo.erase(elmToMoveIt);
                    }
                }
                ++newStart;
            }
        }

        if (newStart <= newEnd) {
            const auto before = newEnd + 1;
            const auto newVnodeStart = std::next(newChildren.begin(), newStart);
            const auto newVnodeEnd = std::next(newChildren.begin(), newEnd);
            if (before < newChildren.size()) {
                const auto beforeVnode = std::next(newChildren.begin(), before);
                addVNodes(parentVnode, *beforeVnode, newVnodeStart, newVnodeEnd);
            } else {
                addVNodes(parentVnode, nullptr, newVnodeStart, newVnodeEnd);
            }
        }

        if (currentStart <= currentEnd) {
            const auto currentVnodeStart = std::next(currentChildren.begin(), currentStart);
            const auto currentVnodeEnd = std::next(currentChildren.begin(), currentEnd);
            removeVNodes(parentVnode, currentVnodeStart, currentVnodeEnd);
        }
    }

    inline void patchVNode(VNodeData*& currentVnode, const VNodeData* const newVnode)
    {
        if (sameVNode(*currentVnode, *newVnode)) {
            if (newVnode->hash & isElementOrFragment) {
                const auto newChildrenNotEmpty = newVnode->hash & hasChildren;
                const auto currentChildrenNotEmpty = currentVnode->hash & hasChildren;

                if (newChildrenNotEmpty && currentChildrenNotEmpty) {
                    updateChildren(
                        currentVnode, currentVnode->children, newVnode->children,
                        0, currentVnode->children.size() - 1, 0, newVnode->children.size() - 1
                    );
                } else if (newChildrenNotEmpty) {
                    addVNodes(currentVnode, nullptr, newVnode->children.cbegin(), newVnode->children.cend());
                } else if (currentChildrenNotEmpty) {
                    removeVNodes(currentVnode, currentVnode->children.begin(), currentVnode->children.end());
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
            const auto& parentVnode = currentVnode->parent;
            const VNodeData* nextSiblingVnode = nullptr;

            unmountVNodeChildren(*currentVnode);
            onEvent(*currentVnode, onUnmount);

            auto mutableNewVnode = const_cast<VNodeData*>(newVnode);

            if (parentVnode) {
                removeNode(*parentVnode, *currentVnode);
                const auto currentNodeIt = std::ranges::find(parentVnode->children, currentVnode);
                *currentNodeIt = mutableNewVnode;
                const auto nextSiblingVnodeIt = std::next(currentNodeIt);
                if (nextSiblingVnodeIt != parentVnode->children.end())
                    nextSiblingVnode = *nextSiblingVnodeIt;
            }

            deleteVNodeData(currentVnode);
            currentVnode = mutableNewVnode;

            createNode(*currentVnode);
            insertBefore(parentVnode, *currentVnode, nextSiblingVnode);
            onEvent(*currentVnode, onMount);
        }
    }
}
