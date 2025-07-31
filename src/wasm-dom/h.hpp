#pragma once

#include "vnode.hpp"

namespace wasmdom::dsl
{
    // can't use comma operator ("s", "s") with string literals
    // it must be std::string, so write ("s", "s"s) or ("s"s, "s")
    using namespace std::string_literals;

    // helper to write VNode((key, val), (key, val)...)
    template <Stringifiable K, Attribute V>
    inline std::pair<K, V> operator,(K&& key, V&& val)
    {
        return { std::forward<K>(key), std::forward<V>(val) };
    }

    // helper to type pointers to data members for operator,
    template <class F>
    inline auto f(F&& f) { return std::function(std::forward<F>(f)); }

    inline VNode t(const std::string& inlineText) { return VNode(text_tag, inlineText); }
    inline VNode fragment() { return VNode(""); }
    inline VNode comment() { return VNode("!"); }
    inline VNode comment(const std::string& text) { return comment()(text); }

#define SEL(X)                                                                                                      \
    inline VNode X() { return VNode(#X); }                                                                          \
    template <Stringifiable... K, Attribute... V>                                                                   \
    inline VNode X(std::pair<K, V>&&... nodeData) { return VNode(#X, std::forward<std::pair<K, V>>(nodeData)...); } \
    inline VNode X(const VNodeAttributes& nodeData) { return VNode(#X, nodeData); }

    SEL(a)
    SEL(b)
    SEL(div)
    SEL(h2)
    SEL(i)
    SEL(input)
    SEL(foreignObject)
    SEL(p)
    SEL(span)
    SEL(style)
    SEL(svg)
    SEL(htemplate)
    SEL(web_component)

    // void elements
    SEL(area)
    SEL(base)
    SEL(br)
    SEL(col)
    SEL(embed)
    SEL(hr)
    SEL(img)
    SEL(keygen)
    SEL(link)
    SEL(meta)
    SEL(param)
    SEL(source)
    SEL(track)
    SEL(wbr)

    // svg elements
    SEL(defs)
    SEL(glyph)
    SEL(g)
    SEL(marker)
    SEL(mask)
    SEL(missing_glyph)
    SEL(pattern)
    SEL(rect)
    SEL(hswitch)
    SEL(symbol)
    SEL(text)
    SEL(desc)
    SEL(metadata)
    SEL(title)

#undef SEL

}
