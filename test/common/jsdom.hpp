#pragma once

#include <emscripten/val.h>

class JSDom
{
public:
    JSDom();

    emscripten::val document() const;
    emscripten::val root() const;
    emscripten::val bodyFirstChild() const;

private:
    emscripten::val _dom;
};
