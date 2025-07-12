#pragma once

#include <emscripten/val.h>

#include <memory>

namespace wasmdom
{
    struct VNode;
    void deleteVNode(const VNode* const vnode);
}

emscripten::val getRoot();
emscripten::val getBodyFirstChild();
emscripten::val getNode(wasmdom::VNode* vnode);

void setupDom();
void reset();

bool onClick(emscripten::val event);

struct VNodeDeleter
{
    void operator()(const wasmdom::VNode* const node) { wasmdom::deleteVNode(node); }
};
using ScopedVNode = std::unique_ptr<wasmdom::VNode, VNodeDeleter>;
