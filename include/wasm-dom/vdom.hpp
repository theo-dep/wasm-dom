#pragma once

#include "wasm-dom/vnodedata.hpp"

namespace emscripten
{
    class val;
}

namespace wasmdom
{
    class VNode;

    class VDom
    {
    public:
        VDom() = default;
        VDom(const emscripten::val& element);
        ~VDom();

        void patch(VNode vnode);

    private:
        VNodeData* _topParentNode{ nullptr };
        VNodeData* _currentNode{ nullptr };
    };
}
