#pragma once

#include "wasm-dom/attribute.hpp"

namespace wasmdom::internals
{
    emscripten::val toJsCallback(const Callback& callback);
}
