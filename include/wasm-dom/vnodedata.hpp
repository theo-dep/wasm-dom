#pragma once

#include "wasm-dom/attribute.hpp"

#include <memory>

namespace emscripten
{
    class val;
}

namespace wasmdom
{
    struct VNodeData
    {
        std::string sel;
        std::string key;
        std::string ns;
        std::size_t hash{ 0 };
        Attrs attrs;
#ifdef __EMSCRIPTEN__
        Props props;
        Callbacks callbacks;
        EventCallbacks eventCallbacks;
        emscripten::val node{ emscripten::val::null() };
#endif

        using Children = std::vector<std::shared_ptr<VNodeData>>;
        Children children;
        VNodeData* parent{ nullptr };
    };
}
