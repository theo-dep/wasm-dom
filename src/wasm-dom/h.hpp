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

#define WASMDOM_DSL_SEL(X)                                                                                          \
    inline VNode X() { return VNode(#X); }                                                                          \
    template <Stringifiable... K, Attribute... V>                                                                   \
    inline VNode X(std::pair<K, V>&&... nodeData) { return VNode(#X, std::forward<std::pair<K, V>>(nodeData)...); } \
    inline VNode X(const VNodeAttributes& nodeData) { return VNode(#X, nodeData); }

#define WASMDOM_DSL_PARENS ()

#define WASMDOM_DSL_EXPAND(...) WASMDOM_DSL_EXPAND4(WASMDOM_DSL_EXPAND4(WASMDOM_DSL_EXPAND4(WASMDOM_DSL_EXPAND4(__VA_ARGS__))))
#define WASMDOM_DSL_EXPAND4(...) WASMDOM_DSL_EXPAND3(WASMDOM_DSL_EXPAND3(WASMDOM_DSL_EXPAND3(WASMDOM_DSL_EXPAND3(__VA_ARGS__))))
#define WASMDOM_DSL_EXPAND3(...) WASMDOM_DSL_EXPAND2(WASMDOM_DSL_EXPAND2(WASMDOM_DSL_EXPAND2(WASMDOM_DSL_EXPAND2(__VA_ARGS__))))
#define WASMDOM_DSL_EXPAND2(...) WASMDOM_DSL_EXPAND1(WASMDOM_DSL_EXPAND1(WASMDOM_DSL_EXPAND1(WASMDOM_DSL_EXPAND1(__VA_ARGS__))))
#define WASMDOM_DSL_EXPAND1(...) __VA_ARGS__

#define WASMDOM_DSL_FOR_EACH(macro, ...) \
    __VA_OPT__(WASMDOM_DSL_EXPAND(WASMDOM_DSL_FOR_EACH_HELPER(macro, __VA_ARGS__)))
#define WASMDOM_DSL_FOR_EACH_HELPER(macro, a1, ...) \
    macro(a1)                                       \
        __VA_OPT__(WASMDOM_DSL_FOR_EACH_AGAIN WASMDOM_DSL_PARENS(macro, __VA_ARGS__))
#define WASMDOM_DSL_FOR_EACH_AGAIN() WASMDOM_DSL_FOR_EACH_HELPER

#define WASMDOM_DSL_ELEMENTS      \
    a,                            \
        b,                        \
        div,                      \
        h2,                       \
        i,                        \
        input,                    \
        foreignObject,            \
        p,                        \
        span,                     \
        style,                    \
        svg,                      \
        htemplate,                \
        web_component,            \
        /* void elements */ area, \
        base,                     \
        br,                       \
        col,                      \
        embed,                    \
        hr,                       \
        img,                      \
        keygen,                   \
        link,                     \
        meta,                     \
        param,                    \
        source,                   \
        track,                    \
        wbr,                      \
        /* svg elements */ defs,  \
        glyph,                    \
        g,                        \
        marker,                   \
        mask,                     \
        missing_glyph,            \
        pattern,                  \
        rect,                     \
        hswitch,                  \
        symbol,                   \
        text,                     \
        desc,                     \
        metadata,                 \
        title

    WASMDOM_DSL_FOR_EACH(WASMDOM_DSL_SEL, WASMDOM_DSL_ELEMENTS)

}
