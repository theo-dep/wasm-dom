#pragma once

#include "handletable.hpp"

#include <emscripten/val.h>

#include <string>

namespace wasmdom::internals::domapi
{
    // Synchronous (creation) operations: deferred to the next flush. They
    // allocate a NodeId on the C++ side and emit the matching CREATE_*
    // opcode; the actual DOM node is created later by wdom_flush.
    NodeId createElement(const std::string& tag);
    NodeId createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
    NodeId createTextNode(const std::string& text);
    NodeId createComment(const std::string& comment);
    NodeId createDocumentFragment();

    // Read-only DOM access. Kept for the public API tests.
    emscripten::val parentNode(const emscripten::val& node);
    emscripten::val nextSibling(const emscripten::val& node);

    // Mutating operations: deferred via the DomOperationQueue.
    void insertBefore(NodeId parent, NodeId newNode, NodeId ref);
    void removeNode(NodeId node);
    void appendChild(NodeId parent, NodeId child);
    void removeAttribute(NodeId node, const std::string& attribute);
    void setAttribute(NodeId node, const std::string& attribute, const std::string& value);
    void setNodeValue(NodeId node, const std::string& text);

    void setProperty(NodeId node, const std::string& name, const emscripten::val& value);
    void ensureEventsObject(NodeId node);
    void setEventsProperty(NodeId node, const std::string& name, const emscripten::val& value);
    void deleteEventsProperty(NodeId node, const std::string& name);
    void addEventListener(NodeId node, const std::string& event, const emscripten::val& listener);
    void removeEventListener(NodeId node, const std::string& event, const emscripten::val& listener);

    // val-taking entry point kept for the public domapi test.
    void removeNode(const emscripten::val& node);
}
