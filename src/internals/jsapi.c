#include "bind.h"

#include <emscripten/em_js.h>
#include <stdint.h>

typedef struct _EM_VAL* EM_VAL;

// JS-side handle table.
// Slot 0 is the null sentinel.
// The table is created lazily by wdom_flush. wdom_get / wdom_drop simply
// look it up. NodeId allocation and refcounting is done in C++; the JS
// table only stores the actual DOM node references.

WASMDOM_EM_JS(EM_VAL, wdom_get, (uint32_t id),
    {
        var t = Module.__wdomTable;
        if (!t) return Emval.toHandle(null);
        var n = t.nodes[id];
        return Emval.toHandle(n == null ? null : n);
    })

WASMDOM_EM_JS(void, wdom_drop, (uint32_t id),
    {
        if (id === 0) return;
        var t = Module.__wdomTable;
        if (!t) return;
        t.nodes[id] = null;
    })

// Opcodes. Must match enum DomOpCode in domoperation.hpp.
//   0  INSERT_BEFORE       parent, node, ref
//   1  REMOVE_NODE         node
//   2  APPEND_CHILD        parent, child
//   3  SET_ATTRIBUTE       node, nameOff, nameLen, valOff, valLen
//   4  REMOVE_ATTRIBUTE    node, nameOff, nameLen
//   5  SET_NODE_VALUE      node, valOff, valLen
//   6  SET_PROPERTY        node, nameOff, nameLen, valIdx
//   7  ENSURE_EVENTS       node
//   8  SET_EVENTS_PROPERTY node, nameOff, nameLen, valIdx
//   9  DELETE_EVENTS_PROP  node, nameOff, nameLen
//  10  ADD_EVENT_LISTENER  node, evOff, evLen, valIdx
//  11  REMOVE_EVENT_LISTNR node, evOff, evLen, valIdx
//  12  CREATE_ELEMENT      id, tagOff, tagLen
//  13  CREATE_ELEMENT_NS   id, nsOff, nsLen, tagOff, tagLen
//  14  CREATE_TEXT         id, textOff, textLen
//  15  CREATE_COMMENT      id, textOff, textLen
//  16  CREATE_FRAGMENT     id
WASMDOM_EM_JS(void, wdom_flush,
    (const uint32_t* cmds, uint32_t cmdsLen, const char* strs, EM_VAL valsHandle, EM_VAL mountsHandle),
    {
        var t = Module.__wdomTable || (Module.__wdomTable = { nodes: [null] });
        var nodes = t.nodes;
        var vals = Emval.toValue(valsHandle);
        var mounts = Emval.toValue(mountsHandle);
        // Register externally-allocated nodes (toVNode) into the handle
        // table before processing opcodes.
        for (var k = 0; k < mounts.length; k += 2) {
            nodes[mounts[k]] = mounts[k + 1];
        }
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
                case 12: { // createElement
                    var id = view[i++], to = view[i++], tl = view[i++];
                    nodes[id] = document.createElement(UTF8ToString(strs + to, tl));
                    break;
                }
                case 13: { // createElementNS
                    var id = view[i++], no = view[i++], nl = view[i++], to = view[i++], tl = view[i++];
                    nodes[id] = document.createElementNS(UTF8ToString(strs + no, nl), UTF8ToString(strs + to, tl));
                    break;
                }
                case 14: { // createTextNode
                    var id = view[i++], to = view[i++], tl = view[i++];
                    nodes[id] = document.createTextNode(UTF8ToString(strs + to, tl));
                    break;
                }
                case 15: { // createComment
                    var id = view[i++], to = view[i++], tl = view[i++];
                    nodes[id] = document.createComment(UTF8ToString(strs + to, tl));
                    break;
                }
                case 16: { // createDocumentFragment
                    var id = view[i++];
                    nodes[id] = document.createDocumentFragment();
                    break;
                }
            }
        }
    })
