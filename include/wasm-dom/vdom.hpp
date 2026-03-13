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

        VNode patch(VNode vnode);

    private:
        VNodeData* _topParentNode{ nullptr };
        VNodeData* _currentNode{ nullptr };

        VDom(const VDom& other) = delete;
        VDom(VDom&& other) = delete;
        VDom& operator=(const VDom& other) = delete;
        VDom& operator=(VDom&& other) = delete;
    };
}
