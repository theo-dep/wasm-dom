#pragma once

#include "wasm-dom/domapi.hpp"
#include "wasm-dom/vnode.hpp"

namespace wasmdom::internals
{
    void patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode);

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

    inline void createNode(VNode& vnode)
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

        for (VNode& child : vnode) {
            createNode(child);
            domapi::appendChild(vnode.node(), child.node());
            onEvent(child, onMount);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);
    }

    inline void addVNodes(const emscripten::val& parentNode, const emscripten::val& beforeNode, Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            createNode(*start);
            domapi::insertBefore(parentNode, start->node(), beforeNode);
            onEvent(*start, onMount);
        }
    }

    inline void removeVNodes(Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            if (*start) {
                onEvent(*start, onUnmount);
                domapi::removeChild(start->node());
            }
        }
    }

    inline void updateChildren(const emscripten::val& parentNode, Children::iterator oldStart, Children::iterator oldEnd, Children::iterator newStart, Children::iterator newEnd, Children::iterator end)
    {
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
                onEvent(*newStart, onUpdate);
                ++oldStart;
                ++newStart;
            } else if (sameVNode(*oldEnd, *newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(*oldEnd, *newEnd, parentNode);
                onEvent(*newEnd, onUpdate);
                --oldEnd;
                --newEnd;
            } else if (sameVNode(*oldStart, *newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(*oldStart, *newEnd, parentNode);
                const emscripten::val nextSiblingOldPtr = domapi::nextSibling(oldEnd->node());
                domapi::insertBefore(parentNode, oldStart->node(), nextSiblingOldPtr);
                onEvent(*oldStart, onMount);
                ++oldStart;
                --newEnd;
            } else if (sameVNode(*oldEnd, *newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(*oldEnd, *newStart, parentNode);
                domapi::insertBefore(parentNode, oldEnd->node(), oldStart->node());
                onEvent(*oldEnd, onMount);
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
                    onEvent(*newStart, onMount);
                } else {
                    const Children::iterator elmToMove = oldKeyTo[newStart->key()];
                    if ((elmToMove->hash() & extractSel) != (newStart->hash() & extractSel)) {
                        createNode(*newStart);
                        domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                        onEvent(*newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart)
                            patchVNode(*elmToMove, *newStart, parentNode);
                        domapi::insertBefore(parentNode, elmToMove->node(), oldStart->node());
                        onEvent(*elmToMove, onMount);
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }
        if (oldStart <= oldEnd || newStart <= newEnd) {
            if (oldStart > oldEnd) {
                addVNodes(parentNode, std::next(newEnd) != end ? std::next(newEnd)->node() : emscripten::val::null(), newStart, newEnd);
            } else {
                removeVNodes(oldStart, oldEnd);
            }
        }
    }
}

inline void wasmdom::internals::patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode)
{
    vnode.setNode(oldVnode.node());
    if (vnode.hash() & isElementOrFragment) {
        const std::size_t childrenNotEmpty = vnode.hash() & hasChildren;
        const std::size_t oldChildrenNotEmpty = oldVnode.hash() & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode.hash() & isFragment ? parentNode : vnode.node(), oldVnode.begin(), std::prev(oldVnode.end()), vnode.begin(), std::prev(vnode.end()), vnode.end());
        } else if (childrenNotEmpty) {
            addVNodes(vnode.hash() & isFragment ? parentNode : vnode.node(), emscripten::val::null(), vnode.begin(), std::prev(vnode.end()));
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode.begin(), std::prev(oldVnode.end()));
        }
        vnode.diff(oldVnode);
    } else if (vnode.sel() != oldVnode.sel()) {
        domapi::setNodeValue(vnode.node(), vnode.sel());
    }
}
