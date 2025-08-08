#include "domapi.hpp"

#include "vnode.hpp"

#include <emscripten.h>

EM_JS(int, addPtr, (emscripten::EM_VAL nodeHandle), {
    var node = Emval.toValue(nodeHandle);
    if (node === null || node === undefined)
        return 0;
    if (node['asmDomPtr'] !== undefined)
        return node['asmDomPtr'];
    Module['lastPtr'] += 1;
    Module['nodes'][Module['lastPtr']] = node;
    return node['asmDomPtr'] = Module['lastPtr'];
});

int wasmdom::domapi::addNode(const emscripten::val& node)
{
    addPtr(node["parentNode"].as_handle());
    addPtr(node["nextSibling"].as_handle());
    return addPtr(node.as_handle());
}

int wasmdom::domapi::createDocumentFragment()
{
    return addPtr(emscripten::val::global("document").call<emscripten::val>("createDocumentFragment").as_handle());
}

void wasmdom::domapi::insertBefore(const VNode& parentNode, const VNode& newNode, const VNode& referenceNode)
{
    insertBefore(parentNode.elm(), newNode.elm(), referenceNode.elm());
}

void wasmdom::domapi::insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr)
{
    if (parentNodePtr == 0 /*|| newNodePtr == 0 || referenceNodePtr == 0*/)
        return;

    EM_ASM(
        { Module['nodes'][$0].insertBefore(Module['nodes'][$1], Module['nodes'][$2]); },
        parentNodePtr, newNodePtr, referenceNodePtr
    );
}
