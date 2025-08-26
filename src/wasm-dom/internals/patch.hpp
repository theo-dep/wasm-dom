#pragma once

#include "wasm-dom/domapi.hpp"
#include "wasm-dom/vnode.hpp"

namespace wasmdom::internals
{
    void patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode);

    inline bool sameVNode(const VNode& vnode1, const VNode& vnode2)
    {
        return
            // compare selector, nodeType and key existence
            ((vnode1.hash() & Flag::id) == (vnode2.hash() & Flag::id)) &&
            // compare keys
            (!(vnode1.hash() & Flag::hasKey) || (vnode1.key() == vnode2.key()));
    }

    inline void createNode(VNode& vnode)
    {
        if (vnode.hash() & Flag::isElement) {
            if (vnode.hash() & Flag::hasNS) {
                vnode.setNode(domapi::createElementNS(vnode.ns(), vnode.sel()));
            } else {
                vnode.setNode(domapi::createElement(vnode.sel()));
            }
        } else if (vnode.hash() & Flag::isText) {
            vnode.setNode(domapi::createTextNode(vnode.sel()));
            return;
        } else if (vnode.hash() & Flag::isFragment) {
            vnode.setNode(domapi::createDocumentFragment());
        } else if (vnode.hash() & Flag::isComment) {
            vnode.setNode(domapi::createComment(vnode.sel()));
            return;
        }

        for (VNode& child : vnode) {
            createNode(child);
            domapi::appendChild(vnode.node(), child.node());
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);
    }

    inline void addVNodes(const emscripten::val& parentNode, const emscripten::val& beforeNode, Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            createNode(*start);
            domapi::insertBefore(parentNode, start->node(), beforeNode);
        }
    }

    inline void removeVNodes(Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            if (*start) {
                domapi::removeChild(start->node());

                if (start->hash() & Flag::hasRef) {
                    start->callbacks().at("ref")(emscripten::val::null());
                }
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
                ++oldStart;
                ++newStart;
            } else if (sameVNode(*oldEnd, *newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(*oldEnd, *newEnd, parentNode);
                --oldEnd;
                --newEnd;
            } else if (sameVNode(*oldStart, *newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(*oldStart, *newEnd, parentNode);
                const emscripten::val nextSiblingOldPtr = domapi::nextSibling(oldEnd->node());
                domapi::insertBefore(parentNode, oldStart->node(), nextSiblingOldPtr);
                ++oldStart;
                --newEnd;
            } else if (sameVNode(*oldEnd, *newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(*oldEnd, *newStart, parentNode);

                domapi::insertBefore(parentNode, oldEnd->node(), oldStart->node());
                --oldEnd;
                ++newStart;
            } else {
                if (!oldKeys) {
                    oldKeys = true;

                    for (Children::iterator begin = oldStart; begin <= oldEnd; ++begin) {
                        if (begin->hash() & Flag::hasKey) {
                            oldKeyTo.emplace(begin->key(), begin);
                        }
                    }
                }
                if (!oldKeyTo.contains(newStart->key())) {
                    createNode(*newStart);
                    domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                } else {
                    const Children::iterator elmToMove = oldKeyTo[newStart->key()];
                    if ((elmToMove->hash() & Flag::extractSel) != (newStart->hash() & Flag::extractSel)) {
                        createNode(*newStart);
                        domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                    } else {
                        if (*elmToMove != *newStart)
                            patchVNode(*elmToMove, *newStart, parentNode);
                        domapi::insertBefore(parentNode, elmToMove->node(), oldStart->node());
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
    if (vnode.hash() & Flag::isElementOrFragment) {
        const int childrenNotEmpty = vnode.hash() & Flag::hasChildren;
        const int oldChildrenNotEmpty = oldVnode.hash() & Flag::hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode.hash() & Flag::isFragment ? parentNode : vnode.node(), oldVnode.begin(), std::prev(oldVnode.end()), vnode.begin(), std::prev(vnode.end()), vnode.end());
        } else if (childrenNotEmpty) {
            addVNodes(vnode.hash() & Flag::isFragment ? parentNode : vnode.node(), emscripten::val::null(), vnode.begin(), std::prev(vnode.end()));
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode.begin(), std::prev(oldVnode.end()));
        }
        vnode.diff(oldVnode);
    } else if (vnode.sel() != oldVnode.sel()) {
        domapi::setNodeValue(vnode.node(), vnode.sel());
    }
}
