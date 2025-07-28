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

    inline VNode t(const std::string& inlineText) { return VNode(text, inlineText); }

#define SEL(X)                                                                                                      \
    inline VNode X() { return VNode(#X); }                                                                          \
    template <Stringifiable... K, Attribute... V>                                                                   \
    inline VNode X(std::pair<K, V>&&... nodeData) { return VNode(#X, std::forward<std::pair<K, V>>(nodeData)...); } \
    inline VNode X(const VNodeAttributes& nodeData) { return VNode(#X, nodeData); }

    SEL(a)
    SEL(div)
    SEL(i)
    SEL(input)
    SEL(span)

#undef SEL

}
