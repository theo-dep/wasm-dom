#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    struct VNode;

    VNode* toVNode(const emscripten::val& node);

}
