#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    namespace domapi
    {

        int addNode(const emscripten::val& node);

    }
}
