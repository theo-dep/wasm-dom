#include "tohtml.hpp"

#include "config.hpp"
#include "vnode.hpp"
#include "vnodeforward.hpp"

#include <emscripten/val.h>

#include <string>
#include <unordered_map>

namespace wasmdom
{

    // All SVG children elements, not in this list, should self-close

    std::unordered_map<std::string, bool> containerElements{
        // http://www.w3.org/TR/SVG/intro.html#TermContainerElement
        { "a", true },
        { "defs", true },
        { "glyph", true },
        { "g", true },
        { "marker", true },
        { "mask", true },
        { "missing-glyph", true },
        { "pattern", true },
        { "svg", true },
        { "switch", true },
        { "symbol", true },
        { "text", true },

        // http://www.w3.org/TR/SVG/intro.html#TermDescriptiveElement
        { "desc", true },
        { "metadata", true },
        { "title", true }
    };

    // http://www.w3.org/html/wg/drafts/html/master/syntax.html#void-elements
    std::unordered_map<std::string, bool> voidElements{
        { "area", true },
        { "base", true },
        { "br", true },
        { "col", true },
        { "embed", true },
        { "hr", true },
        { "img", true },
        { "input", true },
        { "keygen", true },
        { "link", true },
        { "meta", true },
        { "param", true },
        { "source", true },
        { "track", true },
        { "wbr", true }
    };

    // https://developer.mozilla.org/en-US/docs/Web/API/element
    std::unordered_map<std::string, bool> omitProps{
        { "attributes", true },
        { "childElementCount", true },
        { "children", true },
        { "classList", true },
        { "clientHeight", true },
        { "clientLeft", true },
        { "clientTop", true },
        { "clientWidth", true },
        { "currentStyle", true },
        { "firstElementChild", true },
        { "innerHTML", true },
        { "lastElementChild", true },
        { "nextElementSibling", true },
        { "ongotpointercapture", true },
        { "onlostpointercapture", true },
        { "onwheel", true },
        { "outerHTML", true },
        { "previousElementSibling", true },
        { "runtimeStyle", true },
        { "scrollHeight", true },
        { "scrollLeft", true },
        { "scrollLeftMax", true },
        { "scrollTop", true },
        { "scrollTopMax", true },
        { "scrollWidth", true },
        { "tabStop", true },
        { "tagName", true }
    };

    std::string encode(const std::string& data)
    {
        std::string encoded;
        size_t size = data.size();
        encoded.reserve(size);
        for (size_t pos = 0; pos != size; ++pos) {
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

    void appendAttributes(const VNode* const vnode, std::string& html)
    {
        for (auto& it : vnode->data.attrs) {
            html.append(" " + it.first + "=\"" + encode(it.second) + "\"");
        }

        emscripten::val String = emscripten::val::global("String");
        for (auto& it : vnode->data.props) {
            if (!omitProps[it.first]) {
                std::string key = it.first;
                std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                html.append(" " + key + "=\"" + encode(String(it.second).as<std::string>()) + "\"");
            }
        }
    }

    void toHTML(const VNode* const vnode, std::string& html)
    {
        if (!vnode)
            return;

        if (vnode->hash & isText && !vnode->sel.empty()) {
            html.append(encode(vnode->sel));
        } else if (vnode->hash & isComment) {
            html.append("<!--" + vnode->sel + "-->");
        } else if (vnode->hash & isFragment) {
            for (Children::size_type i = 0; i != vnode->children.size(); ++i) {
                toHTML(vnode->children[i], html);
            }
        } else {
            bool isSvg = (vnode->hash & hasNS) && vnode->ns == "http://www.w3.org/2000/svg";
            bool isSvgContainerElement = isSvg && containerElements[vnode->sel];

            html.append("<" + vnode->sel);
            appendAttributes(vnode, html);
            if (isSvg && !isSvgContainerElement) {
                html.append(" /");
            }
            html.append(">");

            if (
                isSvgContainerElement ||
                (!isSvg && !voidElements[vnode->sel])) {

                if (vnode->data.props.count("innerHTML") != 0) {
                    html.append(vnode->data.props.at("innerHTML").as<std::string>());
                } else

                    for (Children::size_type i = 0; i != vnode->children.size(); ++i) {
                        toHTML(vnode->children[i], html);
                    }
                html.append("</" + vnode->sel + ">");
            }
        }
    }

}

std::string wasmdom::toHTML(VNode* const vnode)
{
    std::string html;

    if (vnode)
        vnode->normalize();

    toHTML(vnode, html);

    if (vnode && CLEAR_MEMORY) {
        deleteVNode(vnode);
    }

    return html;
}
