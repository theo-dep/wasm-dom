#pragma once

#include "internals/diff.hpp"
#include "internals/domapi.hpp"

#include <wasm-dom/vnode.hpp>
#include <wasm-dom/vnodedata.hpp>

#include <algorithm>

namespace wasmdom::internals
{
    void patchVNode(const std::shared_ptr<VNodeData>& oldVnode, const std::shared_ptr<VNodeData>& vnode);

    inline void onEvent(const std::shared_ptr<VNodeData>& vnode, const Event& event)
    {
        const EventCallbacks& eventCallbacks{ vnode->eventCallbacks };
        const auto callbackIt = eventCallbacks.find(event);
        if (callbackIt != eventCallbacks.cend()) {
            callbackIt->second(vnode->node);
        }
    }

    inline bool sameVNode(const std::shared_ptr<VNodeData>& vnode1, const std::shared_ptr<VNodeData>& vnode2)
    {
        return
            // compare selector, nodeType and key existence
            ((vnode1->hash & id) == (vnode2->hash & id)) &&
            // compare keys
            (!(vnode1->hash & hasKey) || (vnode1->key == vnode2->key));
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

    inline emscripten::val domSiblingNode(const std::shared_ptr<VNodeData>& vnode)
    {
        if (vnode->hash & isFragment) {
            if (vnode->hash & hasChildren) {
                // a fragment is not added to the DOM, get first child
                return domSiblingNode(*vnode->children.front());
            } else {
                return emscripten::val::null();
            }
        } else {
            return vnode->node;
        }
    }

    inline const VNodeData* nextSibling(const std::shared_ptr<VNodeData>& vnode)
    {
        // Get vnode position in parent children
        if (!vnode->parent) {
            return nullptr;
        }

        const VNodeData::Children::const_iterator vnodeIt{
            std::ranges::find_if(vnode->parent->children, [&vnode](const auto& child) {
                return child.get() == &vnode;
            })
        };
        if (vnodeIt == vnode->parent->children.end()) {
            return nullptr;
        }

        const VNodeData::Children::const_iterator nextSiblingVNodeIt{ std::next(vnodeIt) };
        if (nextSiblingVNodeIt == vnode->parent->children.end()) {
            return nullptr;
        }

        return nextSiblingVNodeIt->get();
    }

    inline emscripten::val nextSiblingNode(const std::shared_ptr<VNodeData>& vnode)
    {
        const VNodeData* next{ nextSibling(vnode) };
        if (!next) {
            return emscripten::val::null();
        }

        return domSiblingNode(*next);
    }

    inline void removeNode(std::shared_ptr<VNodeData>& parentVnode, std::shared_ptr<VNodeData>& vnode)
    {
        std::erase_if(parentVnode.children, [&vnode](const auto& child) {
            return child.get() == &vnode;
        });
        vnode->parent = nullptr;
    }

    inline void removeNode(std::shared_ptr<VNodeData>& vnode)
    {
        if (vnode->hash & isFragment) {
            // a fragment is not added to the DOM, remove its children
            for (const auto& child : vnode->children) {
                removeNode(*child);
            }
        } else {
            if (vnode->parent) {
                removeNode(*vnode->parent, vnode);
                domapi::removeNode(domNode(*vnode->parent), vnode->node);
            }
        }
    }

    inline void createNode(std::shared_ptr<VNodeData>& vnode)
    {
        if (vnode->hash & isElement) {
            if (vnode->hash & hasNS) {
                vnode->node = domapi::createElementNS(vnode->ns, vnode->sel);
            } else {
                vnode->node = domapi::createElement(vnode->sel);
            }
        } else if (vnode->hash & isText) {
            vnode->node = domapi::createTextNode(vnode->sel);
            return;
        } else if (vnode->hash & isFragment) {
            vnode->node = domapi::createDocumentFragment();
        } else if (vnode->hash & isComment) {
            vnode->node = domapi::createComment(vnode->sel);
            return;
        }

        for (const auto& child : vnode->children) {
            createNode(*child);
            domapi::appendChild(vnode->node, child->node);
            onEvent(*child, onMount);
        }

        static const VNodeData emptyNode;
        internals::diff(emptyNode, vnode);
    }

    inline void insertBefore(std::shared_ptr<VNodeData>& parentVnode, std::shared_ptr<VNodeData>& vnode, const VNodeData* referenceVnode)
    {
        const VNodeData::Children::const_iterator vnodeIt{
            std::ranges::find_if(parentVnode.children, [&vnode](const auto& child) {
                return child.get() == &vnode;
            })
        };
        if (vnodeIt != parentVnode.children.end()) {
            parentVnode.children.erase(vnodeIt);
        }

        const VNodeData::Children::const_iterator referenceChildIt{
            std::ranges::find_if(parentVnode.children, [referenceVnode](const auto& child) {
                return child.get() == referenceVnode;
            })
        };
        vnode->parent = &parentVnode;
        parentVnode.children.insert(referenceChildIt, std::shared_ptr<VNodeData>(&vnode));
    }

    inline void insertBefore(
        VNodeData* parentVnode, std::shared_ptr<VNodeData>& vnode, const VNodeData* referenceVnode,
        emscripten::val (*domFunction)(const std::shared_ptr<VNodeData>& vnode)
    )
    {
        if (parentVnode) {
            insertBefore(*parentVnode, vnode, referenceVnode);
            const emscripten::val refNode{ referenceVnode ? domFunction(*referenceVnode) : emscripten::val::null() };
            domapi::insertBefore(domNode(*parentVnode), vnode->node, refNode);
        }
    }

    inline void addVNodes(
        VNodeData* parentVnode, const VNodeData* referenceVnode,
        VNodeData::Children::iterator start, VNodeData::Children::iterator end
    )
    {
        for (; start <= end; ++start) {
            createNode(**start);
            insertBefore(parentVnode, **start, referenceVnode, domSiblingNode);
            onEvent(**start, onMount);
        }
    }

    inline void unmountVNodeChildren(const std::shared_ptr<VNodeData>& vnode)
    {
        for (const auto& child : vnode->children) {
            unmountVNodeChildren(*child);
            onEvent(*child, onUnmount);
        }
    }

    inline void removeVNodes(VNodeData::Children::iterator start, VNodeData::Children::iterator end)
    {
        for (; start <= end; ++start) {
            if (*start) {
                unmountVNodeChildren(**start);
                onEvent(**start, onUnmount);
                removeNode(**start);
            }
        }
    }

    inline void updateChildren(
        VNodeData* parentVnode, VNodeData::Children::iterator oldStart, VNodeData::Children::iterator oldEnd,
        VNodeData::Children::iterator newStart, VNodeData::Children::iterator newEnd, VNodeData::Children::iterator end
    )
    {
        bool oldKeys = false;
        std::unordered_map<std::string, VNodeData::Children::iterator> oldKeyTo;

        while (oldStart <= oldEnd && newStart <= newEnd) {
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
                insertBefore(parentVnode, **newEnd, nextSibling(**oldEnd), nextSiblingNode);
                ++oldStart;
                --newEnd;
            } else if (sameVNode(**oldEnd, **newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(**oldEnd, **newStart);
                insertBefore(parentVnode, **newStart, nextSibling(**oldStart), domSiblingNode);
                --oldEnd;
                ++newStart;
            } else {
                if (!oldKeys) {
                    oldKeys = true;

                    for (VNodeData::Children::iterator begin{ oldStart }; begin <= oldEnd; ++begin) {
                        if ((*begin)->hash & hasKey) {
                            oldKeyTo.emplace((*begin)->key, begin);
                        }
                    }
                }
                if (!oldKeyTo.contains((*newStart)->key)) {
                    createNode(**newStart);
                    insertBefore(parentVnode, **newStart, nextSibling(**oldStart), domSiblingNode);
                    onEvent(**newStart, onMount);
                } else {
                    const VNodeData::Children::iterator elmToMove = oldKeyTo[(*newStart)->key];

                    if (((*elmToMove)->hash & extractSel) != ((*newStart)->hash & extractSel)) {
                        createNode(**newStart);
                        insertBefore(parentVnode, **newStart, nextSibling(**oldStart), domSiblingNode);
                        onEvent(**newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart) {
                            patchVNode(**elmToMove, **newStart);
                            insertBefore(parentVnode, **newStart, nextSibling(**oldStart), domSiblingNode);
                            onEvent(**newStart, onMount);
                        }
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }

        if (newStart <= newEnd) {
            const VNodeData::Children::const_iterator beforeVnode{ std::next(newEnd) };
            if (beforeVnode != end) {
                addVNodes(parentVnode, beforeVnode->get(), newStart, newEnd);
            } else {
                addVNodes(parentVnode, nullptr, newStart, newEnd);
            }
        }

        if (oldStart <= oldEnd) {
            removeVNodes(oldStart, oldEnd);
        }
    }

    inline void patchVNode(std::shared_ptr<VNodeData>& oldVnode, std::shared_ptr<VNodeData>& vnode)
    {
        if (sameVNode(oldVnode, vnode)) {
            vnode->node = oldVnode->node;
            vnode->parent = oldVnode->parent;

            if (vnode->hash & isElementOrFragment) {
                const std::size_t childrenNotEmpty{ vnode->hash & hasChildren };
                const std::size_t oldChildrenNotEmpty{ oldVnode->hash & hasChildren };

                if (childrenNotEmpty && oldChildrenNotEmpty) {
                    updateChildren(
                        oldVnode.get(), oldVnode->children.begin(), std::prev(oldVnode->children.end()),
                        vnode->children.begin(), std::prev(vnode->children.end()), vnode->children.end()
                    );
                } else if (childrenNotEmpty) {
                    addVNodes(oldVnode.get(), nullptr, vnode->children.begin(), std::prev(vnode->children.end()));
                } else if (oldChildrenNotEmpty) {
                    removeVNodes(oldVnode->children.begin(), std::prev(oldVnode->children.end()));
                }

                internals::diff(*oldVnode, *vnode);
            } else if (vnode->sel != oldVnode->sel) {
                domapi::setNodeValue(vnode->node, vnode->sel);
            }

            onEvent(vnode, onUpdate);
        } else {
            createNode(vnode);
            insertBefore(oldVnode->parent, vnode, nextSibling(oldVnode), nextSiblingNode);
            onEvent(vnode, onMount);
            unmountVNodeChildren(oldVnode);
            onEvent(oldVnode, onUnmount);
            removeNode(oldVnode);
        }
    }
}
