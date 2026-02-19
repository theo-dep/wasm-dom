#pragma once

#include "internals/domapi.hpp"

#include <wasm-dom/vnode.hpp>

#include <algorithm>

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

    inline emscripten::val domNode(const VNode& vnode)
    {
        if (!vnode) {
            return emscripten::val::null();
        } else if (vnode.hash() & isFragment && vnode.parent()) {
            // a fragment is not added to the DOM, get parent
            return domNode(vnode.parent());
        } else {
            return vnode.node();
        }
    }

    inline emscripten::val domSiblingNode(const VNode& vnode)
    {
        if (!vnode) {
            return emscripten::val::null();
        } else if (vnode.hash() & isFragment) {
            if (vnode.hash() & hasChildren) {
                // a fragment is not added to the DOM, get first child
                return domSiblingNode(*vnode.begin());
            } else {
                return emscripten::val::null();
            }
        } else {
            return vnode.node();
        }
    }

    inline emscripten::val nextSiblingNode(const VNode& vnode)
    {
        // Get vnode position in parent children
        const VNode& parent{ vnode.parent() };
        if (!parent) {
            return emscripten::val::null();
        }

        const Children::const_iterator vnodeIt{ std::ranges::find(parent, vnode) };
        if (vnodeIt == parent.end()) {
            return emscripten::val::null();
        }

        const Children::const_iterator nextSiblingVNodeIt{ std::next(vnodeIt) };
        if (nextSiblingVNodeIt == parent.end()) {
            return emscripten::val::null();
        }

        return domSiblingNode(*nextSiblingVNodeIt);
    }

    inline void removeNode(const VNode& vnode)
    {
        if (vnode.hash() & isFragment) {
            // a fragment is not added to the DOM, remove its children
            for (const VNode& child : vnode) {
                removeNode(child);
            }
        } else {
            domapi::removeNode(domNode(vnode.parent()), vnode.node());
        }
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

        const emscripten::val node{ vnode.node() };
        for (VNode& child : vnode) {
            createNode(child);
            domapi::appendChild(node, child.node());
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
                    patchVNode(*oldStart, *newStart);
                ++oldStart;
                ++newStart;
            } else if (sameVNode(*oldEnd, *newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(*oldEnd, *newEnd);
                --oldEnd;
                --newEnd;
            } else if (sameVNode(*oldStart, *newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(*oldStart, *newEnd);
                domapi::insertBefore(parentNode, newEnd->node(), nextSiblingNode(*oldEnd));
                ++oldStart;
                --newEnd;
            } else if (sameVNode(*oldEnd, *newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(*oldEnd, *newStart);
                domapi::insertBefore(parentNode, newStart->node(), domSiblingNode(*oldStart));
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
                    domapi::insertBefore(parentNode, newStart->node(), domSiblingNode(*oldStart));
                    onEvent(*newStart, onMount);
                } else {
                    const Children::iterator elmToMove = oldKeyTo[newStart->key()];

                    if ((elmToMove->hash() & extractSel) != (newStart->hash() & extractSel)) {
                        createNode(*newStart);
                        domapi::insertBefore(parentNode, newStart->node(), domSiblingNode(*oldStart));
                        onEvent(*newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart) {
                            patchVNode(*elmToMove, *newStart);
                            domapi::insertBefore(parentNode, newStart->node(), domSiblingNode(*oldStart));
                            onEvent(*newStart, onMount);
                        }
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }

        if (newStart <= newEnd) {
            const emscripten::val before{ std::next(newEnd) != end ? domSiblingNode(*std::next(newEnd)) : emscripten::val::null() };
            addVNodes(parentNode, before, newStart, newEnd);
        }

        if (oldStart <= oldEnd) {
            removeVNodes(oldStart, oldEnd);
        }
    }

    inline void patchVNode(VNode& oldVnode, VNode& vnode)
    {
        if (sameVNode(oldVnode, vnode)) {
            vnode.setNode(oldVnode.node());

            if (vnode.hash() & isElementOrFragment) {
                const std::size_t childrenNotEmpty = vnode.hash() & hasChildren;
                const std::size_t oldChildrenNotEmpty = oldVnode.hash() & hasChildren;

                if (childrenNotEmpty && oldChildrenNotEmpty) {
                    updateChildren(domNode(oldVnode), oldVnode.begin(), std::prev(oldVnode.end()), vnode.begin(), std::prev(vnode.end()), vnode.end());
                } else if (childrenNotEmpty) {
                    addVNodes(domNode(oldVnode), emscripten::val::null(), vnode.begin(), std::prev(vnode.end()));
                } else if (oldChildrenNotEmpty) {
                    removeVNodes(oldVnode.begin(), std::prev(oldVnode.end()));
                }

                vnode.diff(oldVnode);
            } else if (vnode.sel() != oldVnode.sel()) {
                domapi::setNodeValue(vnode.node(), vnode.sel());
            }

            onEvent(vnode, onUpdate);
        } else {
            createNode(vnode);
            domapi::insertBefore(domNode(oldVnode.parent()), vnode.node(), nextSiblingNode(oldVnode));
            onEvent(vnode, onMount);
            unmountVNodeChildren(oldVnode);
            onEvent(oldVnode, onUnmount);
            removeNode(oldVnode);
        }
    }
}
