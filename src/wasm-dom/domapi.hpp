#pragma once

#include <emscripten/val.h>

namespace wasmdom
{

    class VNode;

    namespace domapi
    {
        emscripten::val createElement(const std::string& tag);
        emscripten::val createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
        emscripten::val createTextNode(const std::string& text);
        emscripten::val createComment(const std::string& comment);
        emscripten::val createDocumentFragment();

        void insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode);
        void removeChild(const emscripten::val& child);
        void appendChild(const emscripten::val& parent, const emscripten::val& child);
        void removeAttribute(const emscripten::val& node, const std::string& attribute);
        void setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value);
        void setNodeValue(emscripten::val& node, const std::string& text);

        emscripten::val parentNode(const emscripten::val& node);
        emscripten::val nextSibling(const emscripten::val& node);
    }
}
