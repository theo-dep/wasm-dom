#pragma once

#include "wasm-dom/vnode.hpp"

namespace wasmdom::dsl
{
    // can't use comma operator ("s", "s") with string literals
    // it must be std::string, so write ("s", "s"s) or ("s"s, "s")
    using namespace std::string_literals;

    // helper to write VNode((key, val), (key, val)...)
    template <AttributeKey K, AttributeValue V>
    inline std::pair<K, V> operator,(K&& key, V&& val)
    {
        return { std::forward<K>(key), std::forward<V>(val) };
    }

    // helper to type pointers to data members for operator,
    template <class F>
    inline auto f(F&& f) { return std::function(std::forward<F>(f)); }

    inline VNode t(const std::string& inlineText) { return VNode(text_tag, inlineText); }

    inline VNode comment() { return VNode("!"); }
    inline VNode comment(const std::string& text) { return comment()(text); }

#define WASMDOM_DSL_SEL_NAME(X, N)                                                                                 \
    inline VNode X() { return VNode(N); }                                                                          \
    template <AttributeKey... K, AttributeValue... V>                                                              \
    inline VNode X(std::pair<K, V>&&... nodeData) { return VNode(N, std::forward<std::pair<K, V>>(nodeData)...); } \
    inline VNode X(const VNodeAttributes& nodeData) { return VNode(N, nodeData); }

#define WASMDOM_DSL_SEL(X) WASMDOM_DSL_SEL_NAME(X, #X)

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

    // clang-format off
#define WASMDOM_DSL_ELEMENTS                                        \
        /* Document structure */                                    \
        html,                                                       \
        head,                                                       \
        body,                                                       \
        title,                                                      \
        base,                                                       \
        link,                                                       \
        meta,                                                       \
        style,                                                      \
        script,                                                     \
        noscript,                                                   \
                                                                    \
        /* Sectioning */                                            \
        main,                                                       \
        section,                                                    \
        nav,                                                        \
        article,                                                    \
        aside,                                                      \
        header,                                                     \
        footer,                                                     \
        address,                                                    \
                                                                    \
        /* Headings */                                              \
        h1,                                                         \
        h2,                                                         \
        h3,                                                         \
        h4,                                                         \
        h5,                                                         \
        h6,                                                         \
        hgroup,                                                     \
                                                                    \
        /* Text content */                                          \
        div,                                                        \
        p,                                                          \
        blockquote,                                                 \
        ol,                                                         \
        ul,                                                         \
        li,                                                         \
        dl,                                                         \
        dt,                                                         \
        dd,                                                         \
        figure,                                                     \
        figcaption,                                                 \
        pre,                                                        \
        hr,                                                         \
                                                                    \
        /* Inline text */                                           \
        a,                                                          \
        abbr,                                                       \
        b,                                                          \
        bdi,                                                        \
        bdo,                                                        \
        br,                                                         \
        cite,                                                       \
        code,                                                       \
        data,                                                       \
        dfn,                                                        \
        em,                                                         \
        i,                                                          \
        kbd,                                                        \
        mark,                                                       \
        q,                                                          \
        rb,                                                         \
        rp,                                                         \
        rt,                                                         \
        rtc,                                                        \
        ruby,                                                       \
        s,                                                          \
        samp,                                                       \
        small,                                                      \
        span,                                                       \
        strong,                                                     \
        sub,                                                        \
        sup,                                                        \
        time,                                                       \
        u,                                                          \
        var,                                                        \
        wbr,                                                        \
                                                                    \
        /* Media */                                                 \
        area,                                                       \
        audio,                                                      \
        img,                                                        \
        map,                                                        \
        track,                                                      \
        video,                                                      \
        embed,                                                      \
        iframe,                                                     \
        object,                                                     \
        param,                                                      \
        picture,                                                    \
        portal,                                                     \
        source,                                                     \
                                                                    \
        /* Forms */                                                 \
        form,                                                       \
        label,                                                      \
        input,                                                      \
        button,                                                     \
        select,                                                     \
        datalist,                                                   \
        optgroup,                                                   \
        option,                                                     \
        textarea,                                                   \
        output,                                                     \
        progress,                                                   \
        meter,                                                      \
        fieldset,                                                   \
        legend,                                                     \
                                                                    \
        /* Tables */                                                \
        table,                                                      \
        caption,                                                    \
        colgroup,                                                   \
        col,                                                        \
        tbody,                                                      \
        thead,                                                      \
        tfoot,                                                      \
        tr,                                                         \
        td,                                                         \
        th,                                                         \
                                                                    \
        /* Interactive */                                           \
        details,                                                    \
        summary,                                                    \
        dialog,                                                     \
                                                                    \
        /* Web Components */                                        \
        slot,                                                       \
        /* hTemplate, */                                            \
        /* webComponent, */                                         \
                                                                    \
        /* Deprecated/Obsolete (still in specs) */                  \
        /*  acronym, */                                             \
        /* applet, */                                               \
        /* basefont, */                                             \
        /* big, */                                                  \
        /* center, */                                               \
        /* dir, */                                                  \
        /* font, */                                                 \
        /* frame, */                                                \
        /* frameset, */                                             \
        /* isindex, */                                              \
        /* keygen, */                                               \
        /* listing, */                                              \
        /* marquee, */                                              \
        /* menuitem, */                                             \
        /* multicol, */                                             \
        /* nextid, */                                               \
        /* nobr, */                                                 \
        /* noembed, */                                              \
        /* noframes, */                                             \
        /* plaintext, */                                            \
        /* spacer, */                                               \
        /* strike, */                                               \
        /* tt, */                                                   \
        /* xmp, */                                                  \
                                                                    \
        /* MathML elements commonly used in HTML */                 \
        /* math, */                                                 \
        /* annotation, */                                           \
        /* annotationXml, */                                        \
        /* maction, */                                              \
        /* maligngroup, */                                          \
        /* malignmark, */                                           \
        /* menclose, */                                             \
        /* merror, */                                               \
        /* mfenced, */                                              \
        /* mfrac, */                                                \
        /* mglyph, */                                               \
        /* mi, */                                                   \
        /* mlabeledtr, */                                           \
        /* mlongdiv, */                                             \
        /* mmultiscripts, */                                        \
        /* mn, */                                                   \
        /* mo, */                                                   \
        /* mover, */                                                \
        /* mpadded, */                                              \
        /* mphantom, */                                             \
        /* mroot, */                                                \
        /* mrow, */                                                 \
        /* ms, */                                                   \
        /* mscarries, */                                            \
        /* mscarry, */                                              \
        /* msgroup, */                                              \
        /* mspace, */                                               \
        /* msqrt, */                                                \
        /* msrow, */                                                \
        /* mstack, */                                               \
        /* mstyle, */                                               \
        /* msub, */                                                 \
        /* msup, */                                                 \
        /* msubsup, */                                              \
        /* mtable, */                                               \
        /* mtd, */                                                  \
        /* mtext, */                                                \
        /* mtr, */                                                  \
        /* munder, */                                               \
        /* munderover, */                                           \
        /* semantics, */                                            \
                                                                    \
        /* SVG elements commonly used in HTML */                    \
        svg,                                                        \
        animate,                                                    \
        animateMotion,                                              \
        animateTransform,                                           \
        circle,                                                     \
        clipPath,                                                   \
        /* colorProfile, */                                         \
        defs,                                                       \
        desc,                                                       \
        ellipse,                                                    \
        feBlend,                                                    \
        feColorMatrix,                                              \
        feComponentTransfer,                                        \
        feComposite,                                                \
        feConvolveMatrix,                                           \
        feDiffuseLighting,                                          \
        feDisplacementMap,                                          \
        feDistantLight,                                             \
        feDropShadow,                                               \
        feFlood,                                                    \
        feFuncA,                                                    \
        feFuncB,                                                    \
        feFuncG,                                                    \
        feFuncR,                                                    \
        feGaussianBlur,                                             \
        feImage,                                                    \
        feMerge,                                                    \
        feMergeNode,                                                \
        feMorphology,                                               \
        feOffset,                                                   \
        fePointLight,                                               \
        feSpecularLighting,                                         \
        feSpotLight,                                                \
        feTile,                                                     \
        feTurbulence,                                               \
        filter,                                                     \
        /* fontFace, */                                             \
        /* fontFaceFormat, */                                       \
        /* fontFaceName, */                                         \
        /* fontFaceSrc, */                                          \
        /* fontFaceUri, */                                          \
        foreignObject,                                              \
        g,                                                          \
        glyph,                                                      \
        image,                                                      \
        line,                                                       \
        linearGradient,                                             \
        marker,                                                     \
        mask,                                                       \
        metadata,                                                   \
        /* missingGlyph, */                                         \
        mpath,                                                      \
        path,                                                       \
        pattern,                                                    \
        polygon,                                                    \
        polyline,                                                   \
        radialGradient,                                             \
        rect,                                                       \
        set,                                                        \
        stop,                                                       \
        /* hSwitch, */                                              \
        symbol,                                                     \
        text,                                                       \
        textPath,                                                   \
        tspan,                                                      \
        use,                                                        \
        view
    // clang-format on

    WASMDOM_DSL_FOR_EACH(WASMDOM_DSL_SEL, WASMDOM_DSL_ELEMENTS)

#define WASMDOM_DSL_CONFLICT_ELEMENTS \
    fragment,                         \
        hTemplate, hSwitch,           \
        webComponent,                 \
        missingGlyph,                 \
        fontFace, fontFaceFormat, fontFaceName, fontFaceSrc, fontFaceUri, colorProfile // annotationXml

    // Fragment
    WASMDOM_DSL_SEL_NAME(fragment, "")

    // Elements with naming conflicts resolved
    WASMDOM_DSL_SEL_NAME(hTemplate, "template")
    WASMDOM_DSL_SEL_NAME(hSwitch, "switch")

    // Web Components with dashes
    WASMDOM_DSL_SEL_NAME(webComponent, "web-component")

    // SVG elements with dashes
    WASMDOM_DSL_SEL_NAME(missingGlyph, "missing-glyph")
    WASMDOM_DSL_SEL_NAME(fontFace, "font-face")
    WASMDOM_DSL_SEL_NAME(fontFaceFormat, "font-face-format")
    WASMDOM_DSL_SEL_NAME(fontFaceName, "font-face-name")
    WASMDOM_DSL_SEL_NAME(fontFaceSrc, "font-face-src")
    WASMDOM_DSL_SEL_NAME(fontFaceUri, "font-face-uri")
    WASMDOM_DSL_SEL_NAME(colorProfile, "color-profile")

    // MathML elements with dashes
    // WASMDOM_DSL_SEL_NAME(annotationXml, "annotation-xml")

}
