#include "init.hpp"

#include "emscripten.h"

void wasmdom::init()
{
    EM_ASM(
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

        var nodes = Module['nodes'] = { 0 : null };
        var lastPtr = 0;

        function addPtr(node) {
            // clang-format off
            if (node === null)
                return 0;
            if (node['asmDomPtr'] !== undefined)
                return node['asmDomPtr'];
            // clang-format on
            nodes[++lastPtr] = node;
            return node['asmDomPtr'] = lastPtr;
        };

        Module['addNode'] = function(node) {
                addPtr(node.parentNode);
                addPtr(node.nextSibling);
                return addPtr(node); };
        Module.createElement = function(tagName) { return addPtr(recycler['create'](tagName)); };
        Module.createElementNS = function(namespaceURI, qualifiedName) { return addPtr(recycler['createNS'](qualifiedName, namespaceURI)); };
        Module.createTextNode = function(text) { return addPtr(recycler['createText'](text)); };
        Module.createComment = function(text) { return addPtr(recycler['createComment'](text)); };
        Module.createDocumentFragment = function() { return addPtr(document.createDocumentFragment()); };
        Module.insertBefore = function(parentNodePtr, newNodePtr, referenceNodePtr) { nodes[parentNodePtr].insertBefore(
                                                                                          nodes[newNodePtr],
                                                                                          nodes[referenceNodePtr]); };
        Module.removeChild = function(childPtr) {
                var node = nodes[childPtr];
                if (node === null || node === undefined) return;
                var parent = node.parentNode;
                if (parent !== null) parent.removeChild(node);
                recycler['collect'](node); };
        Module.appendChild = function(parentPtr, childPtr) { nodes[parentPtr].appendChild(nodes[childPtr]); };
        Module.removeAttribute = function(nodePtr, attr) { nodes[nodePtr].removeAttribute(attr); };
        Module.setAttribute = function(nodePtr, attr, value) {
                // xChar = 120
                // colonChar = 58
                if (attr.charCodeAt(0) !== 120) {
                    nodes[nodePtr].setAttribute(attr, value);
                } else if (attr.charCodeAt(3) === 58) {
                    // Assume xml namespace
                    nodes[nodePtr].setAttributeNS('http://www.w3.org/XML/1998/namespace', attr, value);
                } else if (attr.charCodeAt(5) === 58) {
                    // Assume xlink namespace
                    nodes[nodePtr].setAttributeNS('http://www.w3.org/1999/xlink', attr, value);
                } else {
                    nodes[nodePtr].setAttribute(attr, value);
                } };
        Module.parentNode = function(nodePtr) {
                var node = nodes[nodePtr];
                return (
                    node !== null && node !== undefined &&
                    node.parentNode !== null
                ) ? node.parentNode['asmDomPtr'] : 0; };
        Module.nextSibling = function(nodePtr) {
                var node = nodes[nodePtr];
                return (
                    node !== null && node !== undefined &&
                    node.nextSibling !== null
                ) ? node.nextSibling['asmDomPtr'] : 0; };
        Module.setNodeValue = function(nodePtr, text) { nodes[nodePtr].nodeValue = text; };);
}
