#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    namespace domapi
    {
        emscripten::val node(int nodePtr);

        int addNode(const emscripten::val& node);

        int createElement(const std::string& tag);
        int createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
        int createTextNode(const std::string& text);
        int createComment(const std::string& comment);
        int createDocumentFragment();

        void insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr);
        void removeChild(int childPtr);
        void appendChild(int parentPtr, int childPtr);
        void removeAttribute(int nodePtr, const std::string& attribute);
        void setAttribute(int nodePtr, const std::string& attribute, const std::string& value);
        void setNodeValue(int nodePtr, const std::string& text);

        int parentNode(int nodePtr);
        int nextSibling(int nodePtr);
    }
}
