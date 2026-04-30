#pragma once

#include "internals/domapi.hpp"
#include "internals/handletable.hpp"

#include <wasm-dom/vnode.hpp>

#include <unordered_map>

namespace wasmdom::internals
{
    void patchVNode(VNode& oldVnode, VNode& vnode);

    inline void onEvent(const VNode& vnode, const Event& event)
    {
        const EventCallbacks& eventCallbacks = vnode.eventCallbacks();
        const auto callbackIt = eventCallbacks.find(event);
        if (callbackIt != eventCallbacks.cend()) {
            callbackIt->second(vnode.node());
        }
    }

    inline bool sameVNode(const VNode& vnode1, const VNode& vnode2)
    {
        return
            // compare selector, nodeType and key existence
            ((vnode1.hash() & id) == (vnode2.hash() & id)) &&
            // compare keys
            (!(vnode1.hash() & hasKey) || (vnode1.key() == vnode2.key()));
    }

    // The DOM-facing id of a vnode: a non-fragment is its own node; a
    // fragment "is" its parent (since the fragment itself is not in the
    // tree once attached).
    inline NodeId domNodeId(const VNode& vnode)
    {
        if ((vnode.hash() & isFragment) && vnode.parentNodeId() != nullNodeId) {
            return vnode.parentNodeId();
        } else {
            return vnode.nodeId();
        }
    }

    // The first DOM child of a fragment, or the vnode itself otherwise.
    // Used as an `insertBefore` reference.
    inline NodeId domSiblingNodeId(const VNode& vnode)
    {
        if (vnode.hash() & isFragment) {
            if (vnode.hash() & hasChildren) {
                return vnode.begin()->nodeId();
            } else {
                return nullNodeId;
            }
        } else {
            return vnode.nodeId();
        }
    }

    inline void removeNode(const VNode& vnode)
    {
        if (vnode.hash() & isFragment) {
            // a fragment is not added to the DOM, remove its children
            for (const VNode& child : vnode) {
                domapi::removeNode(child.nodeId());
            }
        } else {
            domapi::removeNode(vnode.nodeId());
        }
    }

    inline void createNode(VNode& vnode)
    {
        if (vnode.hash() & isElement) {
            if (vnode.hash() & hasNS) {
                vnode.setNodeId(domapi::createElementNS(vnode.ns(), vnode.sel()));
            } else {
                vnode.setNodeId(domapi::createElement(vnode.sel()));
            }
        } else if (vnode.hash() & isText) {
            vnode.setNodeId(domapi::createTextNode(vnode.sel()));
            return;
        } else if (vnode.hash() & isFragment) {
            vnode.setNodeId(domapi::createDocumentFragment());
        } else if (vnode.hash() & isComment) {
            vnode.setNodeId(domapi::createComment(vnode.sel()));
            return;
        }

        const NodeId childrenParentId{ domNodeId(vnode) };
        for (VNode& child : vnode) {
            createNode(child);
            child.setParentNodeId(childrenParentId);
            domapi::appendChild(vnode.nodeId(), child.nodeId());
            onEvent(child, onMount);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);
    }

    inline void insertBefore(VNode& vnode, NodeId parentId, NodeId beforeId)
    {
        vnode.setParentNodeId(parentId);
        domapi::insertBefore(parentId, vnode.nodeId(), beforeId);
    }

    inline void addVNodes(NodeId parentId, NodeId beforeId, Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            createNode(*start);
            insertBefore(*start, parentId, beforeId);
            onEvent(*start, onMount);
        }
    }

    inline void unmountVNodeChildren(const VNode& vnode)
    {
        for (const VNode& child : vnode) {
            unmountVNodeChildren(child);
            onEvent(child, onUnmount);
        }
    }

    inline void removeVNodes(Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            if (*start) {
                unmountVNodeChildren(*start);
                onEvent(*start, onUnmount);
                removeNode(*start);
            }
        }
    }

    inline void updateChildren(NodeId parentId, Children::iterator oldStart, Children::iterator oldEnd, Children::iterator oldChildrenEnd, Children::iterator newStart, Children::iterator newEnd, Children::iterator end)
    {
        bool oldKeys = false;
        std::unordered_map<std::string, Children::iterator> oldKeyTo;

        // Logical reference for the leftmost VNode of the already-matched
        // right-tail block in the DOM. Replaces a live `nextSibling(oldEnd)`
        // DOM read so the diff stays correct under batched DOM operations.
        Children::iterator rightAnchor = oldChildrenEnd;
        const auto rightAnchorRef = [&]() {
            return rightAnchor != oldChildrenEnd ? domSiblingNodeId(*rightAnchor) : nullNodeId;
        };

        while (oldStart <= oldEnd && newStart <= newEnd) {
            if (!*oldStart) {
                ++oldStart;
            } else if (!*oldEnd) {
                --oldEnd;
            } else if (sameVNode(*oldStart, *newStart)) {
                if (*oldStart != *newStart)
                    patchVNode(*oldStart, *newStart);
                ++oldStart;
                ++newStart;
            } else if (sameVNode(*oldEnd, *newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(*oldEnd, *newEnd);
                rightAnchor = oldEnd;
                --oldEnd;
                --newEnd;
            } else if (sameVNode(*oldStart, *newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(*oldStart, *newEnd);
                domapi::insertBefore(parentId, newEnd->nodeId(), rightAnchorRef());
                rightAnchor = oldStart;
                ++oldStart;
                --newEnd;
            } else if (sameVNode(*oldEnd, *newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(*oldEnd, *newStart);
                domapi::insertBefore(parentId, newStart->nodeId(), domSiblingNodeId(*oldStart));
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
                    insertBefore(*newStart, parentId, domSiblingNodeId(*oldStart));
                    onEvent(*newStart, onMount);
                } else {
                    const Children::iterator elmToMove = oldKeyTo[newStart->key()];

                    if ((elmToMove->hash() & extractSel) != (newStart->hash() & extractSel)) {
                        createNode(*newStart);
                        insertBefore(*newStart, parentId, domSiblingNodeId(*oldStart));
                        onEvent(*newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart) {
                            patchVNode(*elmToMove, *newStart);
                            domapi::insertBefore(parentId, newStart->nodeId(), domSiblingNodeId(*oldStart));
                            onEvent(*newStart, onMount);
                        }
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }

        if (newStart <= newEnd) {
            const NodeId before{ std::next(newEnd) != end ? domSiblingNodeId(*std::next(newEnd)) : nullNodeId };
            addVNodes(parentId, before, newStart, newEnd);
        }

        if (oldStart <= oldEnd) {
            removeVNodes(oldStart, oldEnd);
        }
    }

    inline void patchVNode(VNode& oldVnode, VNode& vnode)
    {
        if (sameVNode(oldVnode, vnode)) {
            vnode.stealNodeId(oldVnode);
            vnode.stealParentNodeId(oldVnode);
            vnode.installedListeners() = oldVnode.installedListeners();

            if (vnode.hash() & isElementOrFragment) {
                const std::size_t childrenNotEmpty = vnode.hash() & hasChildren;
                const std::size_t oldChildrenNotEmpty = oldVnode.hash() & hasChildren;

                if (childrenNotEmpty && oldChildrenNotEmpty) {
                    updateChildren(domNodeId(oldVnode), oldVnode.begin(), std::prev(oldVnode.end()), oldVnode.end(), vnode.begin(), std::prev(vnode.end()), vnode.end());
                } else if (childrenNotEmpty) {
                    addVNodes(domNodeId(oldVnode), nullNodeId, vnode.begin(), std::prev(vnode.end()));
                } else if (oldChildrenNotEmpty) {
                    removeVNodes(oldVnode.begin(), std::prev(oldVnode.end()));
                }

                vnode.diff(oldVnode);
            } else if (vnode.sel() != oldVnode.sel()) {
                domapi::setNodeValue(vnode.nodeId(), vnode.sel());
            }

            onEvent(vnode, onUpdate);
        } else {
            // Replace oldVnode with newly-created vnode at oldVnode's DOM
            // position by inserting the new node in front of oldVnode and
            // then removing oldVnode. Avoids a live `nextSibling` DOM read.
            createNode(vnode);
            insertBefore(vnode, oldVnode.parentNodeId(), domSiblingNodeId(oldVnode));
            onEvent(vnode, onMount);
            unmountVNodeChildren(oldVnode);
            onEvent(oldVnode, onUnmount);
            removeNode(oldVnode);
        }
    }
}
