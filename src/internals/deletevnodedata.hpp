#pragma once

#include <wasm-dom/vnodedata.hpp>

namespace wasmdom::internals
{
    inline void deleteVNodeData(VNodeData* node)
    {
        if (node) {
            for (VNodeData* child : node->children) {
                deleteVNodeData(child);
            }
            delete node;
        }
    }
}
