#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    namespace domapi
    {

        int addNode(const emscripten::val& node);

        void insertBefore(const VNode& parentNode, const VNode& newNode, const VNode& referenceNode);
        void insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr);

    }
}
