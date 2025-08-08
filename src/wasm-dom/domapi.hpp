#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    namespace domapi
    {

        int addNode(const emscripten::val& node);

        int createElement(const std::string& tag);
        int createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
        int createTextNode(const std::string& text);
        int createComment(const std::string& text);
        int createDocumentFragment();

        void insertBefore(const VNode& parentNode, const VNode& newNode, const VNode& referenceNode);
        void insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr);

    }
}
