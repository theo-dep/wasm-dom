#pragma once

#include <emscripten/val.h>

namespace wasmdom
{
    class VNode;
}

emscripten::val getRoot();
emscripten::val getBodyFirstChild();

void setupDom();

bool onClick(emscripten::val event);
