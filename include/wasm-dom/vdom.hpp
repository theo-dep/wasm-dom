#pragma once

#include "wasm-dom/vnodedata.hpp"

#include <functional>
#include <variant>

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

        void patch(VNode vnode);

    private:
        std::shared_ptr<VNodeData> _topParentNode{ nullptr };
        std::variant<std::shared_ptr<VNodeData>, std::reference_wrapper<VNodeData>> _currentNode{ nullptr };
    };
}
