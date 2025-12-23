#pragma once

#include "wasm-dom/vnode.hpp"

namespace emscripten
{
    class val;
}

namespace wasmdom
{

    class VDom
    {
    public:
        VDom() = default;
        VDom(const emscripten::val& element);

        const VNode& patch(VNode vnode);

    private:
        VNode _currentNode = nullptr;
    };

}
