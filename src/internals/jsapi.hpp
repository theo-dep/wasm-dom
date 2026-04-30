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

    void insertBefore(emscripten::EM_VAL parentNode, emscripten::EM_VAL newNode, emscripten::EM_VAL referenceNode);

    void removeChild(emscripten::EM_VAL parentNode, emscripten::EM_VAL child);

    void appendChild(emscripten::EM_VAL parentNode, emscripten::EM_VAL child);

    void removeAttribute(emscripten::EM_VAL node, const char* attribute);

    void setAttributeNS(emscripten::EM_VAL node, const char* ns, const char* attribute, const char* value);

    void setAttribute(emscripten::EM_VAL node, const char* attribute, const char* value);

    // conflict name with https://github.com/emscripten-core/emscripten/blob/main/src/closure-externs/closure-externs.js#L182
    void addEventListener_(emscripten::EM_VAL node, const char* event, emscripten::EM_VAL listener);

    // conflict name with https://github.com/emscripten-core/emscripten/blob/main/src/closure-externs/closure-externs.js#L188
    void removeEventListener_(emscripten::EM_VAL node, const char* event, emscripten::EM_VAL listener);

    // JS-side handle table for batched DOM operations.
    std::uint32_t wdom_alloc(emscripten::EM_VAL handle);
    emscripten::EM_VAL wdom_get(std::uint32_t id);
    void wdom_retain(std::uint32_t id);
    void wdom_drop(std::uint32_t id);
    }
}
