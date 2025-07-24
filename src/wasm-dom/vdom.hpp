#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    class VDom
    {
    public:
        VDom(const emscripten::val& element);
        ~VDom();

        VNode* patch(VNode* vnode);

    private:
        VNode* _currentNode = nullptr;
    };

}
