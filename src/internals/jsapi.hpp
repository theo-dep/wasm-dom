#pragma once

#include <emscripten/val.h>

#include <cstdint>

namespace wasmdom::internals::jsapi
{
    extern "C" {
    emscripten::EM_VAL createElement(const char* name);

    emscripten::EM_VAL createElementNS(const char* ns, const char* name);

    emscripten::EM_VAL createTextNode(const char* text);

    emscripten::EM_VAL createComment(const char* comment);

    emscripten::EM_VAL createDocumentFragment();

    // JS-side handle table for batched DOM operations.
    std::uint32_t wdom_alloc(emscripten::EM_VAL handle);
    emscripten::EM_VAL wdom_get(std::uint32_t id);
    void wdom_retain(std::uint32_t id);
    void wdom_drop(std::uint32_t id);

    // Single-call batch executor. See domoperation.cpp / jsapi.c.
    void wdom_flush(const std::uint32_t* cmds, std::uint32_t cmdsLen, const char* strs, emscripten::EM_VAL valsHandle);
    }
}
