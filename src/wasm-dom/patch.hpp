#pragma once

#include <emscripten/val.h>

#include <vector>

namespace wasmdom
{

    struct VNode;

    extern VNode* currentNode;

    VNode* patch(
        const emscripten::val& element,
        VNode* const vnode);
    VNode* patch(
        VNode* const oldVnode,
        VNode* const vnode);

}
