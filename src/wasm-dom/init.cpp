#include "init.hpp"

#include "emscripten.h"

void wasmdom::init()
{
    EM_ASM({
        Module['eventProxy'] = function(e) { return Module['functionCallback'](this['asmDomVNodeCallbacks'], e.type, e); };

        var recycler = Module['recycler'] = { 'nodes' : {} };
        recycler['create'] = function(name) {
                var list = recycler['nodes'][name.toUpperCase()];
                return list !== undefined && list.pop() || document.createElement(name); };
        recycler['createNS'] = function(name, ns) {
                var list = recycler['nodes'][name.toUpperCase() + ns];
                var node = list !== undefined && list.pop() || document.createElementNS(ns, name);
                node['asmDomNS'] = ns;
                return node; };
        recycler['createText'] = function(text) {
                var list = recycler['nodes']['#TEXT'];
                if (list !== undefined) {
                    var node = list.pop();
                    if (node !== undefined) {
                        node.nodeValue = text;
                        return node;
                    }
                }
                return document.createTextNode(text); };
        recycler['createComment'] = function(comment) {
                var list = recycler['nodes']['#COMMENT'];
                if (list !== undefined) {
                    var node = list.pop();
                    if (node !== undefined) {
                        node.nodeValue = comment;
                        return node;
                    }
                }
                return document.createComment(comment); };
        recycler['collect'] = function(node) {
                // clean
                var i;

                while (i = node.lastChild) {
                    node.removeChild(i);
                    recycler['collect'](i);
                }
                i = node.attributes !== undefined ? node.attributes.length : 0;
                while (i--) node.removeAttribute(node.attributes[i].name);
                node['asmDomVNodeCallbacks'] = undefined;
                if (node['asmDomRaws'] !== undefined) {
                    node['asmDomRaws'].forEach(function(raw) {
                        node[raw] = undefined;
                    });
                    node['asmDomRaws'] = undefined;
                }
                if (node['asmDomEvents'] !== undefined) {
                    Object.keys(node['asmDomEvents']).forEach(function(event) {
                        node.removeEventListener(event, node['asmDomEvents'][event], false);
                    });
                    node['asmDomEvents'] = undefined;
                }
                if (node.nodeValue !== null && node.nodeValue !== "") {
                    node.nodeValue = "";
                }
                Object.keys(node).forEach(function(key) {
                    if (
                        key[0] !== 'a' || key[1] !== 's' || key[2] !== 'm' ||
                        key[3] !== 'D' || key[4] !== 'o' || key[5] !== 'm'
                    ) {
                        node[key] = undefined;
                    }
                });

                // collect
                var name = node.nodeName.toUpperCase();
                if (node['asmDomNS'] !== undefined) name += node.namespaceURI;
                var list = recycler['nodes'][name];
                if (list !== undefined) list.push(node);
                else recycler['nodes'][name] = [node]; };

        Module['nodes'] = { 0 : null };
        Module['lastPtr'] = 0;
    });
}
