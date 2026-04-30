#pragma once

#include "handletable.hpp"

#include <emscripten/val.h>

#include <string>

namespace wasmdom::internals::domapi
{
    // Synchronous (creation) operations: must return a node value used by
    // subsequent operations.
    emscripten::val createElement(const std::string& tag);
    emscripten::val createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
    emscripten::val createTextNode(const std::string& text);
    emscripten::val createComment(const std::string& comment);
    emscripten::val createDocumentFragment();

    // Read-only DOM access. Kept for the public API tests; not used by the
    // internal patch pipeline (which avoids any live DOM reads during diff).
    emscripten::val parentNode(const emscripten::val& node);
    emscripten::val nextSibling(const emscripten::val& node);

    // Mutating operations: deferred via the DomOperationQueue and executed
    // in batch by domQueue().flush(). The queue retains/drops refcounts on
    // the JS handle table so caller-owned NodeIds are not invalidated.
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
