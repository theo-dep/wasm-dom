#pragma once

#include <emscripten/val.h>

namespace wasmdom
{
    class VNode;
}

emscripten::val getRoot();
emscripten::val getBodyFirstChild();
emscripten::val getNode(const wasmdom::VNode* vnode);

void setupDom();

bool onClick(emscripten::val event);
