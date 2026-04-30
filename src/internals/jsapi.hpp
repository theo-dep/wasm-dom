#pragma once

#include <emscripten/val.h>

#include <cstdint>

namespace wasmdom::internals::jsapi
{
    extern "C" {
    // Resolve a NodeId to a fresh emscripten::val. Used outside the patch
    // pipeline (event callbacks, public node() accessor).
    emscripten::EM_VAL wdom_get(std::uint32_t id);

    // Release a NodeId allocated by the patch pipeline. Called at SharedData
    // destruction; not on the hot path.
    void wdom_drop(std::uint32_t id);

    // Single-call batch executor.
    //
    // `cmds` is a flat uint32 buffer of opcodes + inline arguments.
    // `strs` is a side blob holding string payloads (offset/length pairs).
    // `valsHandle` resolves to a JS Array of non-node values (set property /
    // event listener wrappers).
    // `mountsHandle` resolves to a JS Array of (id, val) pairs flattened as
    // [id0, val0, id1, val1, ...] used by OP_MOUNT to register externally
    // owned nodes (e.g. VNode::toVNode) into the handle table.
    void wdom_flush(const std::uint32_t* cmds, std::uint32_t cmdsLen, const char* strs, emscripten::EM_VAL valsHandle, emscripten::EM_VAL mountsHandle);
    }
}
