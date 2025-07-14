#include "tovnode.hpp"

#include "h.hpp"
#include "vnode.hpp"

#include <emscripten/val.h>

#include <algorithm>
#include <string>

wasmdom::VNode* wasmdom::toVNode(const emscripten::val& node)
{
    VNode* vnode;
    int nodeType = node["nodeType"].as<int>();
    // isElement
    if (nodeType == 1) {
        std::string sel = node["tagName"].as<std::string>();
        std::transform(sel.begin(), sel.end(), sel.begin(), ::tolower);

        Attributes attrs;
        int i = node["attributes"]["length"].as<int>();
        while (i--) {
            attrs.emplace(node["attributes"][i]["nodeName"].as<std::string>(), node["attributes"][i]["nodeValue"].as<std::string>());
        }

        Children children;
        i = 0;
        for (int n = node["childNodes"]["length"].as<int>(); i < n; ++i) {
            children.push_back(toVNode(node["childNodes"][i]));
        }

        vnode = h(sel, attrs, children);
        // isText
    } else if (nodeType == 3) {
        vnode = h(node["textContent"].as<std::string>(), true);
        // isComment
    } else if (nodeType == 8) {
        vnode = h("!", node["textContent"].as<std::string>());
    } else {
        vnode = h("");
    }
    vnode->elm = emscripten::val::module_property("addNode")(node).as<int>();
    return vnode;
}
