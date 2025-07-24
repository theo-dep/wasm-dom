#pragma once

#include "vnode.hpp"

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    class VDom
    {
    public:
        VDom(const emscripten::val& element);

        const VNode& patch(const VNode& vnode);

    private:
        VNode _currentNode;
    };

}
