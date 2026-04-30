#include "bind.h"

#include <emscripten/em_js.h>
#include <stdint.h>

typedef struct _EM_VAL* EM_VAL;

WASMDOM_EM_JS(EM_VAL, createElement, (const char* name),
    { return Emval.toHandle(document.createElement(UTF8ToString(name))); })

WASMDOM_EM_JS(EM_VAL, createElementNS, (const char* ns, const char* name),
    { return Emval.toHandle(document.createElementNS(UTF8ToString(ns), UTF8ToString(name))); })

WASMDOM_EM_JS(EM_VAL, createTextNode, (const char* text),
    { return Emval.toHandle(document.createTextNode(UTF8ToString(text))); })

WASMDOM_EM_JS(EM_VAL, createComment, (const char* comment),
    { return Emval.toHandle(document.createComment(UTF8ToString(comment))); })

WASMDOM_EM_JS(EM_VAL, createDocumentFragment, (void),
    { return Emval.toHandle(document.createDocumentFragment()); })

WASMDOM_EM_JS(void, insertBefore, (EM_VAL parentNode, EM_VAL newNode, EM_VAL referenceNode),
    { Emval.toValue(parentNode).insertBefore(Emval.toValue(newNode), Emval.toValue(referenceNode)); })

WASMDOM_EM_JS(void, removeChild, (EM_VAL parentNode, EM_VAL child),
    { Emval.toValue(parentNode).removeChild(Emval.toValue(child)); })

WASMDOM_EM_JS(void, appendChild, (EM_VAL parentNode, EM_VAL child),
    { Emval.toValue(parentNode).appendChild(Emval.toValue(child)); })

WASMDOM_EM_JS(void, removeAttribute, (EM_VAL node, const char * attribute),
    { Emval.toValue(node).removeAttribute(UTF8ToString(attribute)); })

WASMDOM_EM_JS(void, setAttributeNS, (EM_VAL node, const char* ns, const char * attribute, const char * value),
    { Emval.toValue(node).setAttributeNS(UTF8ToString(ns), UTF8ToString(attribute), UTF8ToString(value)); })

WASMDOM_EM_JS(void, setAttribute, (EM_VAL node, const char * attribute, const char * value),
    { Emval.toValue(node).setAttribute(UTF8ToString(attribute), UTF8ToString(value)); })

WASMDOM_EM_JS(void, addEventListener_, (EM_VAL node, const char * event, EM_VAL listener),
    { Emval.toValue(node).addEventListener(UTF8ToString(event), Emval.toValue(listener), false); })

WASMDOM_EM_JS(void, removeEventListener_, (EM_VAL node, const char * event, EM_VAL listener),
    { Emval.toValue(node).removeEventListener(UTF8ToString(event), Emval.toValue(listener), false); })

// JS-side handle table for batched DOM operations.
// Slot 0 is reserved for the null sentinel and is never freed.
WASMDOM_EM_JS(uint32_t, wdom_alloc, (EM_VAL handle),
    {
        var t = Module.__wdomTable || (Module.__wdomTable = { nodes: [null], refs: [Number.MAX_SAFE_INTEGER], freeList: [] });
        var id = t.freeList.length ? t.freeList.pop() : t.nodes.length;
        t.nodes[id] = Emval.toValue(handle);
        t.refs[id] = 1;
        return id;
    })

WASMDOM_EM_JS(EM_VAL, wdom_get, (uint32_t id),
    {
        var t = Module.__wdomTable;
        return Emval.toHandle(t ? t.nodes[id] : null);
    })

WASMDOM_EM_JS(void, wdom_retain, (uint32_t id),
    {
        if (id === 0) return;
        var t = Module.__wdomTable;
        if (!t) return;
        ++t.refs[id];
    })

WASMDOM_EM_JS(void, wdom_drop, (uint32_t id),
    {
        if (id === 0) return;
        var t = Module.__wdomTable;
        if (!t) return;
        if (--t.refs[id] === 0) {
            t.nodes[id] = null;
            t.freeList.push(id);
        }
    })
