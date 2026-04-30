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

// Single-call DOM batch executor. `cmds` is a flat uint32 buffer in WASM
// linear memory, `strs` is a side blob holding all string payloads, and
// `valsHandle` resolves to a JS Array of values used by setProperty /
// setEventsProperty / addEventListener / removeEventListener ops.
WASMDOM_EM_JS(void, wdom_flush, (const uint32_t* cmds, uint32_t cmdsLen, const char* strs, EM_VAL valsHandle),
    {
        var t = Module.__wdomTable;
        var nodes = t.nodes;
        var vals = Emval.toValue(valsHandle);
        var view = HEAPU32.subarray(cmds >> 2, (cmds >> 2) + cmdsLen);
        var i = 0;
        while (i < cmdsLen) {
            var op = view[i++];
            switch (op) {
                case 0: { // insertBefore
                    var p = view[i++], n = view[i++], r = view[i++];
                    var pn = nodes[p];
                    if (pn != null) pn.insertBefore(nodes[n], nodes[r] || null);
                    break;
                }
                case 1: { // removeNode
                    var n = view[i++];
                    var nd = nodes[n];
                    if (nd != null) {
                        var par = nd.parentNode;
                        if (par) par.removeChild(nd);
                    }
                    break;
                }
                case 2: { // appendChild
                    var p = view[i++], c = view[i++];
                    nodes[p].appendChild(nodes[c]);
                    break;
                }
                case 3: { // setAttribute
                    var n = view[i++], no = view[i++], nl = view[i++], vo = view[i++], vl = view[i++];
                    var name = UTF8ToString(strs + no, nl);
                    var val = UTF8ToString(strs + vo, vl);
                    if (name.indexOf('xml:') === 0) nodes[n].setAttributeNS('http://www.w3.org/XML/1998/namespace', name, val);
                    else if (name.indexOf('xlink:') === 0) nodes[n].setAttributeNS('http://www.w3.org/1999/xlink', name, val);
                    else nodes[n].setAttribute(name, val);
                    break;
                }
                case 4: { // removeAttribute
                    var n = view[i++], no = view[i++], nl = view[i++];
                    nodes[n].removeAttribute(UTF8ToString(strs + no, nl));
                    break;
                }
                case 5: { // setNodeValue
                    var n = view[i++], vo = view[i++], vl = view[i++];
                    nodes[n].nodeValue = UTF8ToString(strs + vo, vl);
                    break;
                }
                case 6: { // setProperty
                    var n = view[i++], no = view[i++], nl = view[i++], vi = view[i++];
                    nodes[n][UTF8ToString(strs + no, nl)] = vals[vi];
                    break;
                }
                case 7: { // ensureEventsObject
                    var n = view[i++];
                    if (nodes[n].wasmDomEvents === undefined) nodes[n].wasmDomEvents = {};
                    break;
                }
                case 8: { // setEventsProperty
                    var n = view[i++], no = view[i++], nl = view[i++], vi = view[i++];
                    nodes[n].wasmDomEvents[UTF8ToString(strs + no, nl)] = vals[vi];
                    break;
                }
                case 9: { // deleteEventsProperty
                    var n = view[i++], no = view[i++], nl = view[i++];
                    delete nodes[n].wasmDomEvents[UTF8ToString(strs + no, nl)];
                    break;
                }
                case 10: { // addEventListener
                    var n = view[i++], no = view[i++], nl = view[i++], vi = view[i++];
                    nodes[n].addEventListener(UTF8ToString(strs + no, nl), vals[vi], false);
                    break;
                }
                case 11: { // removeEventListener
                    var n = view[i++], no = view[i++], nl = view[i++], vi = view[i++];
                    nodes[n].removeEventListener(UTF8ToString(strs + no, nl), vals[vi], false);
                    break;
                }
            }
        }
    })
