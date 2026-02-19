#pragma once

#include <string>

namespace emscripten
{
    class val;
}

namespace wasmdom::internals::domapi
{
    emscripten::val createElement(const std::string& tag);
    emscripten::val createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
    emscripten::val createTextNode(const std::string& text);
    emscripten::val createComment(const std::string& comment);
    emscripten::val createDocumentFragment();

    void insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode);
    void removeNode(const emscripten::val& parentNode, const emscripten::val& node);
    void appendChild(const emscripten::val& parent, const emscripten::val& child);
    void removeAttribute(const emscripten::val& node, const std::string& attribute);
    void setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value);
    void setNodeValue(emscripten::val& node, const std::string& text);
}
