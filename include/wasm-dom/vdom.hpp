#pragma once

#include "wasm-dom/vnodedata.hpp"

#include <functional>
#include <optional>
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
        std::optional<VNodeData> _topParentNode{ std::nullopt };
        std::variant<std::optional<VNodeData>, std::reference_wrapper<VNodeData>> _currentNode{ std::nullopt };
    };
}
