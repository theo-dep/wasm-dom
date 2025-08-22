#pragma once

#include "wasm-dom/internals/utils.hpp"
#include "wasm-dom/vnode.hpp"

#include <algorithm>
#include <array>

namespace wasmdom::internals
{

    // All SVG children elements, not in this list, should self-close

    static constexpr inline std::array containerElements{
        // http://www.w3.org/TR/SVG/intro.html#TermContainerElement
        "a",
        "defs",
        "glyph",
        "g",
        "marker",
        "mask",
        "missing-glyph",
        "pattern",
        "svg",
        "switch",
        "symbol",
        "text",

        // http://www.w3.org/TR/SVG/intro.html#TermDescriptiveElement
        "desc",
        "metadata",
        "title"
    };

    // http://www.w3.org/html/wg/drafts/html/master/syntax.html#void-elements
    static constexpr inline std::array voidElements{
        "area",
        "base",
        "br",
        "col",
        "embed",
        "hr",
        "img",
        "input",
        //"keygen",
        "link",
        "meta",
        "param",
        "source",
        "track",
        "wbr"
    };

    // https://developer.mozilla.org/en-US/docs/Web/API/element
    static constexpr inline std::array omitProps{
        "attributes",
        "childElementCount",
        "children",
        "classList",
        "clientHeight",
        "clientLeft",
        "clientTop",
        "clientWidth",
        "currentStyle",
        "firstElementChild",
        "innerHTML",
        "lastElementChild",
        "nextElementSibling",
        "ongotpointercapture",
        "onlostpointercapture",
        "onwheel",
        "outerHTML",
        "previousElementSibling",
        "runtimeStyle",
        "scrollHeight",
        "scrollLeft",
        "scrollLeftMax",
        "scrollTop",
        "scrollTopMax",
        "scrollWidth",
        "tabStop",
        "tagName"
    };

    inline std::string encode(const std::string& data)
    {
        std::string encoded;
        std::size_t size = data.size();
        encoded.reserve(size);
        for (std::size_t pos = 0; pos != size; ++pos) {
            switch (data[pos]) {
                case '&':
                    encoded.append("&amp;");
                    break;
                case '\"':
                    encoded.append("&quot;");
                    break;
                case '\'':
                    encoded.append("&apos;");
                    break;
                case '<':
                    encoded.append("&lt;");
                    break;
                case '>':
                    encoded.append("&gt;");
                    break;
                case '`':
                    encoded.append("&#96;");
                    break;
                default:
                    encoded.append(&data[pos], 1);
                    break;
            }
        }
        return encoded;
    }

    inline void appendAttributes(const VNode& vnode, std::string& html)
    {
        for (const auto& [key, val] : vnode.attrs()) {
            html.append(" " + key + "=\"" + encode(val) + "\"");
        }

        static const emscripten::val String = emscripten::val::global("String");

        for (const auto& [key, val] : vnode.props()) {
            if (std::ranges::find(omitProps, key) == omitProps.cend()) {
                std::string lowerKey(key);
                lower(lowerKey);
                html.append(" " + lowerKey + "=\"" + encode(String(val).as<std::string>()) + "\"");
            }
        }
    }

    inline void toHTML(const VNode& vnode, std::string& html)
    {
        if (!vnode)
            return;

        if (vnode.hash() & isText && !vnode.sel().empty()) {
            html.append(encode(vnode.sel()));
        } else if (vnode.hash() & isComment) {
            html.append("<!--" + vnode.sel() + "-->");
        } else if (vnode.hash() & isFragment) {
            for (const VNode& child : vnode) {
                toHTML(child, html);
            }
        } else {
            bool isSvg = (vnode.hash() & hasNS) && vnode.ns() == "http://www.w3.org/2000/svg";
            bool isSvgContainerElement = isSvg && std::ranges::find(containerElements, vnode.sel()) != containerElements.cend();

            html.append("<" + vnode.sel());
            appendAttributes(vnode, html);
            if (isSvg && !isSvgContainerElement) {
                html.append(" /");
            }
            html.append(">");

            if (isSvgContainerElement ||
                (!isSvg && std::ranges::find(voidElements, vnode.sel()) == voidElements.cend())) {

                const auto propsIt = vnode.props().find("innerHTML");
                if (propsIt != vnode.props().cend()) {
                    html.append(propsIt->second.as<std::string>());
                } else {
                    for (const VNode& child : vnode) {
                        toHTML(child, html);
                    }
                }
                html.append("</" + vnode.sel() + ">");
            }
        }
    }

}
