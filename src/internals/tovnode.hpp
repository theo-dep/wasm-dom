#pragma once

#include <wasm-dom/vnode.hpp>

#include <emscripten/val.h>

namespace wasmdom::internals
{
    inline VNode toVNode(const emscripten::val& node)
    {
        if (node.isNull()) {
            return nullptr;
        }

        VNode vnode(nullptr);

        const int nodeType = node["nodeType"].as<int>();
        switch (nodeType) {
            case 1: // isElement
            {
                const std::string sel{ internals::lower(node["tagName"].as<std::string>()) };

                VNodeAttributes data;
                for (int i : std::views::iota(0, node["attributes"]["length"].as<int>())) {
                    data.attrs.emplace(node["attributes"][i]["nodeName"].as<std::string>(), node["attributes"][i]["nodeValue"].as<std::string>());
                }

                Children children;
                for (int i : std::views::iota(0, node["childNodes"]["length"].as<int>())) {
                    children.push_back(toVNode(node["childNodes"][i]));
                }

                vnode = VNode(sel, data)(children);
            } break;

            case 3: // isText
                vnode = VNode(text_tag, node["textContent"].as<std::string>());
                break;

            case 8: // isComment
                vnode = VNode("!")(node["textContent"].as<std::string>());
                break;

            default: // isDocumentFragment
            {
                // if fragment is not added to the DOM yet
                Children children;
                for (int i : std::views::iota(0, node["childElementCount"].as<int>())) {
                    children.push_back(toVNode(node["children"][i]));
                }

                vnode = VNode("")(children);
            }
        }

        vnode.setNode(node);
        return vnode;
    }
}
