#pragma once

#include <emscripten/val.h>

#include <vector>

namespace wasmdom
{

    struct VNode;

    VNode* patch(
        const emscripten::val& element,
        VNode* const vnode);
    VNode* patch(
        VNode* const oldVnode,
        VNode* const vnode);

#ifdef ASMDOM_TEST
    void reset();
#endif

}
