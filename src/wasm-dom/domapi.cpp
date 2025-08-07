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
