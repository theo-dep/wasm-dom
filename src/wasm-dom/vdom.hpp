#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    class VDom
    {
    public:
        VNode* patch(const emscripten::val& element, VNode* vnode);

        VNode* patch(VNode* oldVnode, VNode* vnode);

    private:
        VNode* _currentNode = nullptr;
    };

}
