#pragma once

#include <wasm-dom/vnodedata.hpp>

namespace wasmdom::internals
{
    inline void deleteVNodeData(VNodeData* const node)
    {
        if (node) {
            for (VNodeData* const child : node->children) {
                deleteVNodeData(child);
            }
            delete node;
        }
    }
}
