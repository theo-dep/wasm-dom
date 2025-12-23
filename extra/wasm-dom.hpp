// =============================================================================
// Single Header Library
// Auto-generated from multiple source files
// Project: wasm-dom
// =============================================================================
#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <emscripten/bind.h>
#include <emscripten/em_js.h>
#include <emscripten/val.h>
#include <functional>
#include <memory>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------
// include/wasm-dom/attribute.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

    template <typename T>
    concept Stringifiable = std::convertible_to<T, std::string>;

    template <typename T>
    concept StringAttribute = Stringifiable<T>;

    template <typename T>
    concept ValAttribute = std::convertible_to<T, emscripten::val>;

    template <typename T>
    concept CallbackAttribute = requires(T f) {
        { f(std::declval<emscripten::val>()) } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept EventCallbackAttribute = requires(T f) {
        { f(std::declval<emscripten::val>()) } -> std::same_as<void>;
    };

    struct Event
    {
        std::size_t e{};
        bool operator==(const Event&) const;
    };
    static inline constexpr Event onMount{ 0 };
    static inline constexpr Event onUpdate{ 1 };
    static inline constexpr Event onUnmount{ 2 };

    struct EventHash
    {
        std::size_t operator()(const Event& e) const;
    };

    template <typename T>
    concept EventAttribute = std::convertible_to<T, Event>;

    template <typename T>
    concept AttributeKey = Stringifiable<T> || EventAttribute<T>;

    template <typename T>
    concept AttributeValue = StringAttribute<T> || ValAttribute<T> || CallbackAttribute<T> || EventCallbackAttribute<T>;

    using Callback = std::function<bool(emscripten::val)>;
    using EventCallback = std::function<void(emscripten::val)>;

    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;
    using EventCallbacks = std::unordered_map<Event, EventCallback, EventHash>;

    struct VNodeAttributes
    {
        Attrs attrs;
        Props props;
        Callbacks callbacks;
        EventCallbacks eventCallbacks;
    };

    namespace internals
    {
        template <AttributeKey K, AttributeValue V>
        inline void attributeToVNode(VNodeAttributes& attributes, std::pair<K, V>&& attribute)
        {
            auto&& [key, value]{ attribute };
            if constexpr (EventAttribute<K> && EventCallbackAttribute<V>) {
                attributes.eventCallbacks.emplace(key, value);
            } else if constexpr (StringAttribute<V>) {
                attributes.attrs.emplace(key, value);
            } else if constexpr (ValAttribute<V>) {
                attributes.props.emplace(key, value);
            } else if constexpr (CallbackAttribute<V>) {
                attributes.callbacks.emplace(key, value);
            } else {
                static_assert(false, "Type not supported");
            }
        }
    }

    template <AttributeKey... K, AttributeValue... V>
    inline VNodeAttributes attributesToVNode(std::pair<K, V>&&... attributes)
    {
        VNodeAttributes vnodeAttributes;
        (internals::attributeToVNode(vnodeAttributes, std::forward<std::pair<K, V>>(attributes)), ...);
        return vnodeAttributes;
    }

}

// -----------------------------------------------------------------------------
// include/wasm-dom/conf.h
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// include/wasm-dom/attribute.inl.hpp
// -----------------------------------------------------------------------------
inline bool wasmdom::Event::operator==(const Event&) const = default;

inline std::size_t wasmdom::EventHash::operator()(const Event& e) const
{
    return std::hash<std::size_t>{}(e.e);
}

// -----------------------------------------------------------------------------
// include/wasm-dom/vnode.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

    class VNode;
    using Children = std::vector<VNode>;

    enum VNodeFlags
    {
        // NodeType
        isElement = 1,
        isText = 1 << 1,
        isComment = 1 << 2,
        isFragment = 1 << 3,

        // flags
        hasKey = 1 << 4,
        hasText = 1 << 5,
        hasAttrs = 1 << 6,
        hasProps = 1 << 7,
        hasCallbacks = 1 << 8,
        hasEventCallbacks = 1 << 9,
        hasDirectChildren = 1 << 10,
        hasNS = 1 << 11,
        isNormalized = 1 << 12,

        // masks
        hasChildren = hasDirectChildren | hasText,
        isElementOrFragment = isElement | isFragment,
        nodeType = isElement | isText | isComment | isFragment,
        removeNodeType = ~0 ^ nodeType,
        extractSel = ~0 << 13,
        id = extractSel | hasKey | nodeType
    };

    struct text_tag_t
    {
    };
    static inline constexpr text_tag_t text_tag{};

    class VNode
    {
        struct SharedData
        {
            std::string sel;
            std::string key;
            std::string ns;
            std::size_t hash{ 0 };
            VNodeAttributes data;
            emscripten::val node{ emscripten::val::null() };
            Children children;
        };

    public:
        VNode(std::nullptr_t);
        VNode(const std::string& nodeSel);
        VNode(text_tag_t, const std::string& nodeText);
        template <AttributeKey... K, AttributeValue... V>
        VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData);
        VNode(const std::string& nodeSel, const VNodeAttributes& nodeData);

        VNode& operator()(const std::string& nodeText);

        VNode& operator()(const VNode& child);
        VNode& operator()(const Children& nodeChildren);
        VNode& operator()(std::initializer_list<VNode> nodeChildren);

        const Attrs& attrs() const;
        const Props& props() const;
        const Callbacks& callbacks() const;
        const EventCallbacks& eventCallbacks() const;

        const std::string& sel() const;
        const std::string& key() const;
        const std::string& ns() const;
        std::size_t hash() const;
        const emscripten::val& node() const;
        emscripten::val& node();

        void setNode(const emscripten::val& node);

        void normalize();

        operator bool() const;
        bool operator!() const;
        bool operator==(const VNode& other) const;

        std::string toHTML() const;

        void diff(const VNode& other);

        static VNode toVNode(const emscripten::val& node);

        Children::iterator begin();
        Children::iterator end();
        Children::const_iterator begin() const;
        Children::const_iterator end() const;

    private:
        void normalize(bool injectSvgNamespace);

        // contains selector for elements and fragments, text for comments and textNodes
        std::shared_ptr<SharedData> _data = nullptr;
    };
}

template <wasmdom::AttributeKey... K, wasmdom::AttributeValue... V>
inline wasmdom::VNode::VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData)
    : VNode(nodeSel)
{
    _data->data = attributesToVNode(std::forward<std::pair<K, V>>(nodeData)...);
}

// -----------------------------------------------------------------------------
// include/wasm-dom/h.hpp
// -----------------------------------------------------------------------------
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
    inline VNode fragment() { return VNode(""); }
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
    hTemplate, hSwitch,               \
        webComponent,                 \
        missingGlyph,                 \
        fontFace, fontFaceFormat, fontFaceName, fontFaceSrc, fontFaceUri, colorProfile // annotationXml

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

// -----------------------------------------------------------------------------
// include/wasm-dom/vdom.hpp
// -----------------------------------------------------------------------------
namespace emscripten
{
    class val;
}

namespace wasmdom
{

    class VDom
    {
    public:
        VDom() = default;
        VDom(const emscripten::val& element);

        const VNode& patch(VNode vnode);

    private:
        VNode _currentNode = nullptr;
    };

}

// -----------------------------------------------------------------------------
// include/wasm-dom/vnode.inl.hpp
// -----------------------------------------------------------------------------
inline wasmdom::VNode::VNode(std::nullptr_t) {}

inline wasmdom::VNode::VNode(const std::string& nodeSel)
    : _data(std::make_shared<SharedData>())
{
    _data->sel = nodeSel;
}

inline wasmdom::VNode::VNode(text_tag_t, const std::string& nodeText)
    : _data(std::make_shared<SharedData>())
{
    normalize();
    _data->sel = nodeText;
    // replace current type with text type
    _data->hash = (_data->hash & removeNodeType) | isText;
}

inline wasmdom::VNode::VNode(const std::string& nodeSel, const VNodeAttributes& nodeData)
    : VNode(nodeSel)
{
    _data->data = nodeData;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(const std::string& nodeText)
{
    normalize();
    if (_data->hash & isComment) {
        _data->sel = nodeText;
    } else {
        _data->children.emplace_back(text_tag, nodeText);
        _data->hash |= hasText;
    }
    return *this;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(const VNode& child)
{
    _data->children.push_back(child);
    return *this;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(const Children& nodeChildren)
{
    _data->children = nodeChildren;
    return *this;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(std::initializer_list<VNode> nodeChildren)
{
    _data->children = nodeChildren;
    return *this;
}

inline const wasmdom::Attrs& wasmdom::VNode::attrs() const { return _data->data.attrs; }

inline const wasmdom::Props& wasmdom::VNode::props() const { return _data->data.props; }

inline const wasmdom::Callbacks& wasmdom::VNode::callbacks() const { return _data->data.callbacks; }

inline const wasmdom::EventCallbacks& wasmdom::VNode::eventCallbacks() const { return _data->data.eventCallbacks; }

inline const std::string& wasmdom::VNode::sel() const { return _data->sel; }

inline const std::string& wasmdom::VNode::key() const { return _data->key; }

inline const std::string& wasmdom::VNode::ns() const { return _data->ns; }

inline std::size_t wasmdom::VNode::hash() const { return _data->hash; }

inline const emscripten::val& wasmdom::VNode::node() const { return _data->node; }

inline emscripten::val& wasmdom::VNode::node() { return _data->node; }

inline void wasmdom::VNode::setNode(const emscripten::val& node) { _data->node = node; }

inline void wasmdom::VNode::normalize() { normalize(false); }

inline wasmdom::VNode::operator bool() const { return _data != nullptr; }

inline bool wasmdom::VNode::operator!() const { return _data == nullptr; }

inline bool wasmdom::VNode::operator==(const VNode& other) const { return _data == other._data; }

inline wasmdom::Children::iterator wasmdom::VNode::begin() { return _data->children.begin(); }

inline wasmdom::Children::iterator wasmdom::VNode::end() { return _data->children.end(); }

inline wasmdom::Children::const_iterator wasmdom::VNode::begin() const { return _data->children.begin(); }

inline wasmdom::Children::const_iterator wasmdom::VNode::end() const { return _data->children.end(); }

// -----------------------------------------------------------------------------
// src/internals/bind.h
// -----------------------------------------------------------------------------
// for single header use, see https://github.com/emscripten-core/emscripten/issues/25219
// use WASMDOM_EM_JS instead of EM_JS in library mode for unicity
#define _WASMDOM_EM_JS(ret, c_name, js_name, params, code)                          \
    _EM_BEGIN_CDECL                                                                 \
    ret c_name params EM_IMPORT(js_name);                                           \
    __attribute__((weak, visibility("hidden"))) void* __em_js_ref_##c_name =        \
        (void*)&c_name;                                                             \
    EMSCRIPTEN_KEEPALIVE                                                            \
    __attribute__((weak, section("em_js"), aligned(1))) char __em_js__##js_name[] = \
        #params "<::>" code;                                                        \
    _EM_END_CDECL

#define WASMDOM_EM_JS(ret, name, params, ...) _WASMDOM_EM_JS(ret, name, name, params, #__VA_ARGS__)

// -----------------------------------------------------------------------------
// src/internals/domapi.hpp
// -----------------------------------------------------------------------------
namespace emscripten
{
    class val;
}

namespace wasmdom::internals::domapi
{
    emscripten::val createElement(const std::string& tag);
    emscripten::val createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
    emscripten::val createTextNode(const std::string& text);
    emscripten::val createComment(const std::string& comment);
    emscripten::val createDocumentFragment();

    void insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode);
    void removeChild(const emscripten::val& child);
    void appendChild(const emscripten::val& parent, const emscripten::val& child);
    void removeAttribute(const emscripten::val& node, const std::string& attribute);
    void setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value);
    void setNodeValue(emscripten::val& node, const std::string& text);

    emscripten::val parentNode(const emscripten::val& node);
    emscripten::val nextSibling(const emscripten::val& node);
}

// -----------------------------------------------------------------------------
// src/internals/domkeys.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{

    static inline constexpr const char* nodeKeyPrefix = "wasmDom";
    static inline constexpr const char* nodeEventsKey = "wasmDomEvents";
    static inline constexpr const char* nodeCallbackName = "wasmdomCallback";
    static inline constexpr const char* nodeNSKey = "wasmDomNS";
    static inline constexpr const char* nodeRawsKey = "wasmDomRaws";

}

// -----------------------------------------------------------------------------
// build/src/internals/jsapi.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals::jsapi
{

    WASMDOM_EM_JS(emscripten::EM_VAL, createElement, (const char* name),
    { return Emval.toHandle(document.createElement(UTF8ToString(name))); })

    WASMDOM_EM_JS(emscripten::EM_VAL, createElementNS, (const char* ns, const char* name),
    { return Emval.toHandle(document.createElementNS(UTF8ToString(ns), UTF8ToString(name))); })

    WASMDOM_EM_JS(emscripten::EM_VAL, createTextNode, (const char* text),
    { return Emval.toHandle(document.createTextNode(UTF8ToString(text))); })

    WASMDOM_EM_JS(emscripten::EM_VAL, createComment, (const char* comment),
    { return Emval.toHandle(document.createComment(UTF8ToString(comment))); })

    WASMDOM_EM_JS(emscripten::EM_VAL, createDocumentFragment, (void),
    { return Emval.toHandle(document.createDocumentFragment()); })

    WASMDOM_EM_JS(void, insertBefore, (emscripten::EM_VAL parentNode, emscripten::EM_VAL newNode, emscripten::EM_VAL referenceNode),
    { Emval.toValue(parentNode).insertBefore(Emval.toValue(newNode), Emval.toValue(referenceNode)); })

    WASMDOM_EM_JS(void, removeChild, (emscripten::EM_VAL parentNode, emscripten::EM_VAL child),
    { Emval.toValue(parentNode).removeChild(Emval.toValue(child)); })

    WASMDOM_EM_JS(void, appendChild, (emscripten::EM_VAL parentNode, emscripten::EM_VAL child),
    { Emval.toValue(parentNode).appendChild(Emval.toValue(child)); })

    WASMDOM_EM_JS(void, removeAttribute, (emscripten::EM_VAL node, const char * attribute),
    { Emval.toValue(node).removeAttribute(UTF8ToString(attribute)); })

    WASMDOM_EM_JS(void, setAttributeNS, (emscripten::EM_VAL node, const char* ns, const char * attribute, const char * value),
    { Emval.toValue(node).setAttributeNS(UTF8ToString(ns), UTF8ToString(attribute), UTF8ToString(value)); })

    WASMDOM_EM_JS(void, setAttribute, (emscripten::EM_VAL node, const char * attribute, const char * value),
    { Emval.toValue(node).setAttribute(UTF8ToString(attribute), UTF8ToString(value)); })

    WASMDOM_EM_JS(void, addEventListener_, (emscripten::EM_VAL node, const char * event, emscripten::EM_VAL listener),
    { Emval.toValue(node).addEventListener(UTF8ToString(event), Emval.toValue(listener), false); })

    WASMDOM_EM_JS(void, removeEventListener_, (emscripten::EM_VAL node, const char * event, emscripten::EM_VAL listener),
    { Emval.toValue(node).removeEventListener(UTF8ToString(event), Emval.toValue(listener), false); })

}

// -----------------------------------------------------------------------------
// src/internals/wire.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    emscripten::val toJsCallback(const Callback& callback);
}

// -----------------------------------------------------------------------------
// src/internals/diff.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    inline void diffAttrs(const VNode& oldVnode, const VNode& vnode)
    {
        const Attrs& oldAttrs = oldVnode.attrs();
        const Attrs& attrs = vnode.attrs();

        const emscripten::val& node = vnode.node();

        for (const auto& [key, _] : oldAttrs) {
            if (!attrs.contains(key)) {
                domapi::removeAttribute(node, key);
            }
        }

        for (const auto& [key, val] : attrs) {
            const auto oldAttrsIt = oldAttrs.find(key);
            if (oldAttrsIt == oldAttrs.cend() || oldAttrsIt->second != val) {
                domapi::setAttribute(node, key, val);
            }
        }
    }

    inline void diffProps(const VNode& oldVnode, VNode& vnode)
    {
        const Props& oldProps = oldVnode.props();
        const Props& props = vnode.props();

        const emscripten::val nodeRaws = emscripten::val::array(
            props | std::views::keys | std::ranges::to<std::vector<std::string>>()
        );

        emscripten::val& node = vnode.node();
        node.set(nodeRawsKey, nodeRaws);

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                node.set(key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            const auto oldPropsIt = oldProps.find(key);
            if (oldPropsIt == oldProps.cend() ||
                !val.strictlyEquals(oldPropsIt->second) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(node[key]))) {
                node.set(key, val);
            }
        }
    }

    inline std::string formatEventKey(const std::string& key)
    {
        static constexpr std::string_view eventPrefix = "on";
        if (key.starts_with(eventPrefix))
            return key.substr(eventPrefix.size());
        return key;
    }

    inline void diffCallbacks(const VNode& oldVnode, VNode& vnode)
    {
        const Callbacks& oldCallbacks = oldVnode.callbacks();
        const Callbacks& callbacks = vnode.callbacks();

        emscripten::val& node = vnode.node();

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            eventKey = formatEventKey(key);
            jsapi::removeEventListener_(node.as_handle(), eventKey.c_str(), node[nodeEventsKey][eventKey].as_handle());
            node[nodeEventsKey].delete_(eventKey);
        }

        if (node[nodeEventsKey].isUndefined()) {
            node.set(nodeEventsKey, emscripten::val::object());
        }

        for (auto& [key, val] : callbacks) {
            eventKey = formatEventKey(key);
            const emscripten::val jsCallback = toJsCallback(val);
            const emscripten::val functorAdapter = jsCallback["opcall"].call<emscripten::val>("bind", jsCallback);
            jsapi::addEventListener_(node.as_handle(), eventKey.c_str(), functorAdapter.as_handle());
            node[nodeEventsKey].set(eventKey, functorAdapter);
        }
    }

}

// -----------------------------------------------------------------------------
// src/internals/domfactory.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    class DomRecycler;

    struct DomFactory
    {
        static inline emscripten::val create(DomRecycler&, const std::string& name)
        {
            return emscripten::val::take_ownership(jsapi::createElement(name.c_str()));
        }
        static inline emscripten::val createNS(DomRecycler&, const std::string& name, const std::string& ns)
        {
            emscripten::val node = emscripten::val::take_ownership(jsapi::createElementNS(ns.c_str(), name.c_str()));
            node.set(nodeNSKey, ns);
            return node;
        }
        static inline emscripten::val createText(DomRecycler&, const std::string& text)
        {
            return emscripten::val::take_ownership(jsapi::createTextNode(text.c_str()));
        }
        static inline emscripten::val createComment(DomRecycler&, const std::string& comment)
        {
            return emscripten::val::take_ownership(jsapi::createComment(comment.c_str()));
        }

        static inline void collect(DomRecycler&, emscripten::val /*node*/) {} // // LCOV_EXCL_LINE
    };
}

// -----------------------------------------------------------------------------
// src/internals/domrecycler.hpp
// -----------------------------------------------------------------------------
namespace emscripten
{
    class val;
}

namespace wasmdom::internals
{
    struct DomFactoryVTable;
    struct DomFactory;
    struct DomRecyclerFactory;

    class DomRecycler
    {
    public:
        DomRecycler(bool useWasmGC);

        emscripten::val create(const std::string& name);
        emscripten::val createNS(const std::string& name, const std::string& ns);
        emscripten::val createText(const std::string& text);
        emscripten::val createComment(const std::string& comment);

        void collect(emscripten::val node);

        // valid if no garbage collector
        std::vector<emscripten::val> nodes(const std::string& name) const;

    private:
        friend struct internals::DomFactory;
        friend struct internals::DomRecyclerFactory;
        const internals::DomFactoryVTable* _factory;

        std::unordered_map<std::string, std::vector<emscripten::val>> _nodes;
    };

}

// -----------------------------------------------------------------------------
// src/internals/utils.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    inline void lower(std::string& str)
    {
        static const auto tolower{
            [](unsigned char c) -> std::string::value_type {
                return std::tolower(c);
            }
        };
        std::ranges::copy(std::views::transform(str, tolower), str.begin());
    }

    inline std::string upper(const std::string& str)
    {
        static const auto toupper{
            [](unsigned char c) -> std::string::value_type {
                return std::toupper(c);
            }
        };
        std::string upperStr = str;
        std::ranges::copy(std::views::transform(str, toupper), upperStr.begin());
        return upperStr;
    }
}

// -----------------------------------------------------------------------------
// src/internals/domrecyclerfactory.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    struct DomRecyclerFactory
    {
        static emscripten::val create(DomRecycler& recycler, const std::string& name);
        static emscripten::val createNS(DomRecycler& recycler, const std::string& name, const std::string& ns);
        static emscripten::val createText(DomRecycler& recycler, const std::string& text);
        static emscripten::val createComment(DomRecycler& recycler, const std::string& comment);

        static void collect(DomRecycler& recycler, emscripten::val node);
    };
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::create(DomRecycler& recycler, const std::string& name)
{
    std::vector<emscripten::val>& list = recycler._nodes[upper(name)];

    if (list.empty())
        return DomFactory::create(recycler, name);

    emscripten::val node = list.back();
    list.pop_back();
    return node;
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::createNS(DomRecycler& recycler, const std::string& name, const std::string& ns)
{
    std::vector<emscripten::val>& list = recycler._nodes[upper(name) + ns];

    if (list.empty())
        return DomFactory::createNS(recycler, name, ns);

    emscripten::val node = list.back();
    list.pop_back();

    node.set(nodeNSKey, ns);
    return node;
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::createText(DomRecycler& recycler, const std::string& text)
{
    constexpr const char* textKey = "#TEXT";
    std::vector<emscripten::val>& list = recycler._nodes[textKey];

    if (list.empty())
        return DomFactory::createText(recycler, text);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", text);
    return node;
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::createComment(DomRecycler& recycler, const std::string& comment)
{
    constexpr const char* commentKey = "#COMMENT";
    std::vector<emscripten::val>& list = recycler._nodes[commentKey];

    if (list.empty())
        return DomFactory::createComment(recycler, comment);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", comment);
    return node;
}

inline void wasmdom::internals::DomRecyclerFactory::collect(DomRecycler& recycler, emscripten::val node)
{
    // clean
    for (emscripten::val child = node["lastChild"]; !child.isNull(); child = node["lastChild"]) {
        jsapi::removeChild(node.as_handle(), child.as_handle());
        collect(recycler, child);
    }

    if (!node["attributes"].isUndefined()) {
        for (int i : std::views::iota(0, node["attributes"]["length"].as<int>())) {
            jsapi::removeAttribute(node.as_handle(), node["attributes"][i]["name"].as<std::string>().c_str());
        }
    }

    if (!node[nodeRawsKey].isUndefined()) {
        for (int i : std::views::iota(0, node[nodeRawsKey]["length"].as<int>())) {
            node.set(node[nodeRawsKey][i], emscripten::val::undefined());
        }
        node.set(nodeRawsKey, emscripten::val::undefined());
    }

    static const emscripten::val objectEntries = emscripten::val::global("Object")["entries"];

    if (!node[nodeEventsKey].isUndefined()) {
        const emscripten::val entries = objectEntries(node[nodeEventsKey]);
        for (int i : std::views::iota(0, entries["length"].as<int>())) {
            const emscripten::val pair = entries[i];
            const emscripten::val event = pair[0];
            const emscripten::val eventCallback = pair[1];
            jsapi::removeEventListener_(node.as_handle(), event.as<std::string>().c_str(), eventCallback.as_handle());
        }
        node.set(nodeEventsKey, emscripten::val::undefined());
    }

    if (!node["nodeValue"].isNull() && node["nodeValue"].isString()) {
        node.set("nodeValue", std::string{});
    }

    static const emscripten::val objectKeys = emscripten::val::global("Object")["keys"];

    const emscripten::val nodeKeys = objectKeys(node);
    for (int i : std::views::iota(0, nodeKeys["length"].as<int>())) {
        const std::string key = nodeKeys[i].as<std::string>();
        if (!key.starts_with(nodeKeyPrefix)) {
            node.set(key, emscripten::val::undefined());
        }
    }

    // collect
    std::string nodeName = upper(node["nodeName"].as<std::string>());
    if (!node[nodeNSKey].isUndefined()) {
        nodeName += node["namespaceURI"].as<std::string>();
    }

    std::vector<emscripten::val>& list = recycler._nodes[nodeName];
    list.push_back(node);
}

// -----------------------------------------------------------------------------
// src/internals/patch.hpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    void patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode);

    inline void onEvent(const VNode& vnode, const Event& event)
    {
        const EventCallbacks& eventCallbacks = vnode.eventCallbacks();
        const auto callbackIt = eventCallbacks.find(event);
        if (callbackIt != eventCallbacks.cend()) {
            callbackIt->second(vnode.node());
        }
    }

    inline bool sameVNode(const VNode& vnode1, const VNode& vnode2)
    {
        return
            // compare selector, nodeType and key existence
            ((vnode1.hash() & id) == (vnode2.hash() & id)) &&
            // compare keys
            (!(vnode1.hash() & hasKey) || (vnode1.key() == vnode2.key()));
    }

    inline void createNode(VNode& vnode)
    {
        if (vnode.hash() & isElement) {
            if (vnode.hash() & hasNS) {
                vnode.setNode(domapi::createElementNS(vnode.ns(), vnode.sel()));
            } else {
                vnode.setNode(domapi::createElement(vnode.sel()));
            }
        } else if (vnode.hash() & isText) {
            vnode.setNode(domapi::createTextNode(vnode.sel()));
            return;
        } else if (vnode.hash() & isFragment) {
            vnode.setNode(domapi::createDocumentFragment());
        } else if (vnode.hash() & isComment) {
            vnode.setNode(domapi::createComment(vnode.sel()));
            return;
        }

        for (VNode& child : vnode) {
            createNode(child);
            domapi::appendChild(vnode.node(), child.node());
            onEvent(child, onMount);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);
    }

    inline void addVNodes(const emscripten::val& parentNode, const emscripten::val& beforeNode, Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            createNode(*start);
            domapi::insertBefore(parentNode, start->node(), beforeNode);
            onEvent(*start, onMount);
        }
    }

    inline void unmountVNodeChildren(const VNode& vnode)
    {
        for (const VNode& child : vnode) {
            unmountVNodeChildren(child);
            onEvent(child, onUnmount);
        }
    }

    inline void removeVNodes(Children::iterator start, Children::iterator end)
    {
        for (; start <= end; ++start) {
            if (*start) {
                unmountVNodeChildren(*start);
                onEvent(*start, onUnmount);
                domapi::removeChild(start->node());
            }
        }
    }

    inline void updateChildren(const emscripten::val& parentNode, Children::iterator oldStart, Children::iterator oldEnd, Children::iterator newStart, Children::iterator newEnd, Children::iterator end)
    {
        bool oldKeys = false;
        std::unordered_map<std::string, Children::iterator> oldKeyTo;

        while (oldStart <= oldEnd && newStart <= newEnd) {
            if (!*oldStart) {
                ++oldStart;
            } else if (!*oldEnd) {
                --oldEnd;
            } else if (sameVNode(*oldStart, *newStart)) {
                if (*oldStart != *newStart)
                    patchVNode(*oldStart, *newStart, parentNode);
                onEvent(*newStart, onUpdate);
                ++oldStart;
                ++newStart;
            } else if (sameVNode(*oldEnd, *newEnd)) {
                if (*oldEnd != *newEnd)
                    patchVNode(*oldEnd, *newEnd, parentNode);
                onEvent(*newEnd, onUpdate);
                --oldEnd;
                --newEnd;
            } else if (sameVNode(*oldStart, *newEnd)) {
                if (*oldStart != *newEnd)
                    patchVNode(*oldStart, *newEnd, parentNode);
                const emscripten::val nextSiblingOldPtr = domapi::nextSibling(oldEnd->node());
                domapi::insertBefore(parentNode, newEnd->node(), nextSiblingOldPtr);
                onEvent(*newEnd, onMount);
                ++oldStart;
                --newEnd;
            } else if (sameVNode(*oldEnd, *newStart)) {
                if (*oldEnd != *newStart)
                    patchVNode(*oldEnd, *newStart, parentNode);
                domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                onEvent(*newStart, onMount);
                --oldEnd;
                ++newStart;
            } else {
                if (!oldKeys) {
                    oldKeys = true;

                    for (Children::iterator begin = oldStart; begin <= oldEnd; ++begin) {
                        if (begin->hash() & hasKey) {
                            oldKeyTo.emplace(begin->key(), begin);
                        }
                    }
                }
                if (!oldKeyTo.contains(newStart->key())) {
                    createNode(*newStart);
                    domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                    onEvent(*newStart, onMount);
                } else {
                    const Children::iterator elmToMove = oldKeyTo[newStart->key()];
                    if ((elmToMove->hash() & extractSel) != (newStart->hash() & extractSel)) {
                        createNode(*newStart);
                        domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                        onEvent(*newStart, onMount);
                    } else {
                        if (*elmToMove != *newStart)
                            patchVNode(*elmToMove, *newStart, parentNode);
                        domapi::insertBefore(parentNode, newStart->node(), oldStart->node());
                        onEvent(*newStart, onMount);
                        *elmToMove = nullptr;
                    }
                }
                ++newStart;
            }
        }
        if (oldStart <= oldEnd || newStart <= newEnd) {
            if (oldStart > oldEnd) {
                addVNodes(parentNode, std::next(newEnd) != end ? std::next(newEnd)->node() : emscripten::val::null(), newStart, newEnd);
            } else {
                removeVNodes(oldStart, oldEnd);
            }
        }
    }
}

inline void wasmdom::internals::patchVNode(VNode& oldVnode, VNode& vnode, const emscripten::val& parentNode)
{
    vnode.setNode(oldVnode.node());
    if (vnode.hash() & isElementOrFragment) {
        const std::size_t childrenNotEmpty = vnode.hash() & hasChildren;
        const std::size_t oldChildrenNotEmpty = oldVnode.hash() & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode.hash() & isFragment ? parentNode : vnode.node(), oldVnode.begin(), std::prev(oldVnode.end()), vnode.begin(), std::prev(vnode.end()), vnode.end());
        } else if (childrenNotEmpty) {
            addVNodes(vnode.hash() & isFragment ? parentNode : vnode.node(), emscripten::val::null(), vnode.begin(), std::prev(vnode.end()));
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode.begin(), std::prev(oldVnode.end()));
        }
        vnode.diff(oldVnode);
    } else if (vnode.sel() != oldVnode.sel()) {
        domapi::setNodeValue(vnode.node(), vnode.sel());
    }
}

// -----------------------------------------------------------------------------
// src/internals/tohtml.hpp
// -----------------------------------------------------------------------------
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
        const std::size_t size = data.size();
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
            const bool isSvg = (vnode.hash() & hasNS) && vnode.ns() == "http://www.w3.org/2000/svg";
            const bool isSvgContainerElement = isSvg && std::ranges::find(containerElements, vnode.sel()) != containerElements.cend();

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

// -----------------------------------------------------------------------------
// src/attribute.cpp
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// src/internals/domapi.cpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    inline DomRecycler& recycler()
    {
        static DomRecycler recycler(true);
        return recycler;
    }
}

inline emscripten::val wasmdom::internals::domapi::createElement(const std::string& tag)
{
    return recycler().create(tag);
}

inline emscripten::val wasmdom::internals::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return recycler().createNS(qualifiedName, namespaceURI);
}

inline emscripten::val wasmdom::internals::domapi::createTextNode(const std::string& text)
{
    return recycler().createText(text);
}

inline emscripten::val wasmdom::internals::domapi::createComment(const std::string& comment)
{
    return recycler().createComment(comment);
}

inline emscripten::val wasmdom::internals::domapi::createDocumentFragment()
{
    return emscripten::val::take_ownership(jsapi::createDocumentFragment());
}

inline void wasmdom::internals::domapi::insertBefore(const emscripten::val& parentNode, const emscripten::val& newNode, const emscripten::val& referenceNode)
{
    if (parentNode.isNull() || parentNode.isUndefined())
        return;

    jsapi::insertBefore(parentNode.as_handle(), newNode.as_handle(), referenceNode.as_handle());
}

inline void wasmdom::internals::domapi::removeChild(const emscripten::val& child)
{
    if (child.isNull() || child.isUndefined())
        return;

    const emscripten::val parentNode(child["parentNode"]);
    if (!parentNode.isNull())
        jsapi::removeChild(parentNode.as_handle(), child.as_handle());

    recycler().collect(child);
}

inline void wasmdom::internals::domapi::appendChild(const emscripten::val& parent, const emscripten::val& child)
{
    jsapi::appendChild(parent.as_handle(), child.as_handle());
}

inline void wasmdom::internals::domapi::removeAttribute(const emscripten::val& node, const std::string& attribute)
{
    jsapi::removeAttribute(node.as_handle(), attribute.c_str());
}

inline void wasmdom::internals::domapi::setAttribute(const emscripten::val& node, const std::string& attribute, const std::string& value)
{
    if (attribute.starts_with("xml:")) {
        jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/XML/1998/namespace", attribute.c_str(), value.c_str());
    } else if (attribute.starts_with("xlink:")) {
        jsapi::setAttributeNS(node.as_handle(), "http://www.w3.org/1999/xlink", attribute.c_str(), value.c_str());
    } else {
        jsapi::setAttribute(node.as_handle(), attribute.c_str(), value.c_str());
    }
}

inline void wasmdom::internals::domapi::setNodeValue(emscripten::val& node, const std::string& text)
{
    node.set("nodeValue", text);
}

inline emscripten::val wasmdom::internals::domapi::parentNode(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined() && !node["parentNode"].isNull())
        return node["parentNode"];
    return emscripten::val::null();
}

inline emscripten::val wasmdom::internals::domapi::nextSibling(const emscripten::val& node)
{
    if (!node.isNull() && !node.isUndefined() && !node["nextSibling"].isNull())
        return node["nextSibling"];
    return emscripten::val::null();
}

// -----------------------------------------------------------------------------
// src/internals/domrecycler.cpp
// -----------------------------------------------------------------------------
namespace wasmdom::internals
{
    WASMDOM_EM_JS(bool, testGC, (), {
        // https://github.com/GoogleChromeLabs/wasm-feature-detect/blob/main/src/detectors/gc/index.js
        return WebAssembly.validate(new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0, 1, 5, 1, 95, 1, 120, 0]));
    })

    struct DomFactoryVTable
    {
        emscripten::val (*create)(DomRecycler&, const std::string&) = nullptr;
        emscripten::val (*createNS)(DomRecycler&, const std::string&, const std::string&) = nullptr;
        emscripten::val (*createText)(DomRecycler&, const std::string&) = nullptr;
        emscripten::val (*createComment)(DomRecycler&, const std::string&) = nullptr;
        void (*collect)(DomRecycler&, emscripten::val) = nullptr;
    };

    template <typename T>
    inline consteval DomFactoryVTable makeDomVTable()
    {
        return {
            &T::create,
            &T::createNS,
            &T::createText,
            &T::createComment,
            &T::collect
        };
    }

    static inline constexpr DomFactoryVTable domFactoryVTable = makeDomVTable<internals::DomFactory>();
    static inline constexpr DomFactoryVTable domRecyclerFactoryVTable = makeDomVTable<internals::DomRecyclerFactory>();
}

inline wasmdom::internals::DomRecycler::DomRecycler(bool useWasmGC)
    : _factory{ testGC() && useWasmGC ? &domFactoryVTable : &domRecyclerFactoryVTable }
{
}

inline emscripten::val wasmdom::internals::DomRecycler::create(const std::string& name)
{
    return _factory->create(*this, name);
}

inline emscripten::val wasmdom::internals::DomRecycler::createNS(const std::string& name, const std::string& ns)
{
    return _factory->createNS(*this, name, ns);
}

inline emscripten::val wasmdom::internals::DomRecycler::createText(const std::string& text)
{
    return _factory->createText(*this, text);
}

inline emscripten::val wasmdom::internals::DomRecycler::createComment(const std::string& comment)
{
    return _factory->createComment(*this, comment);
}

inline void wasmdom::internals::DomRecycler::collect(emscripten::val node)
{
    _factory->collect(*this, node);
}

inline std::vector<emscripten::val> wasmdom::internals::DomRecycler::nodes(const std::string& name) const
{
    const auto nodeIt = _nodes.find(name);
    if (nodeIt != _nodes.cend())
        return nodeIt->second;
    return {};
}

// -----------------------------------------------------------------------------
// src/internals/wire.cpp
// -----------------------------------------------------------------------------
inline emscripten::val wasmdom::internals::toJsCallback(const Callback& callback)
{
    return emscripten::val(callback);
}

// Callback binding template specialization, address are owned by VNode
// see https://github.com/emscripten-core/emscripten/issues/25399
namespace emscripten::internal
{
    template <>
    struct BindingType<wasmdom::Callback>
    {
        using WireType = const wasmdom::Callback*;

        static inline WireType toWireType(const wasmdom::Callback& c, rvp::default_tag)
        {
            return &c;
        }

        static inline const wasmdom::Callback& fromWireType(WireType wt, rvp::default_tag)
        {
            return *wt;
        }
    };
}

namespace wasmdom::internals
{
    // in single header mode, the binding function must be registered only once
    // see https://github.com/emscripten-core/emscripten/issues/25219
    __attribute__((weak)) emscripten::internal::InitFunc wasmdomInitEventProxyFunc([] {
        emscripten::class_<Callback>(nodeCallbackName)
            .constructor<>()
            .function("opcall", &Callback::operator());
    });
}

// -----------------------------------------------------------------------------
// src/vdom.cpp
// -----------------------------------------------------------------------------
inline wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode.normalize();
}

inline const wasmdom::VNode& wasmdom::VDom::patch(VNode vnode)
{
    if (!_currentNode || !vnode || _currentNode == vnode)
        return _currentNode;

    vnode.normalize();

    if (internals::sameVNode(_currentNode, vnode)) {
        internals::patchVNode(_currentNode, vnode, _currentNode.node());
        internals::onEvent(vnode, onUpdate);
    } else {
        internals::createNode(vnode);
        const emscripten::val parentNode = internals::domapi::parentNode(_currentNode.node());
        const emscripten::val nextSiblingNode = internals::domapi::nextSibling(_currentNode.node());
        internals::domapi::insertBefore(parentNode, vnode.node(), nextSiblingNode);
        internals::onEvent(vnode, onMount);
        internals::unmountVNodeChildren(_currentNode);
        internals::onEvent(_currentNode, onUnmount);
        internals::domapi::removeChild(_currentNode.node());
    }

    _currentNode = vnode;

    return _currentNode;
}

// -----------------------------------------------------------------------------
// src/vnode.cpp
// -----------------------------------------------------------------------------
inline void wasmdom::VNode::normalize(bool injectSvgNamespace)
{
    if (!_data)
        return;

    if (!(_data->hash & isNormalized)) {
        const auto attrsIt = _data->data.attrs.find("key");
        if (attrsIt != _data->data.attrs.cend()) {
            _data->hash |= hasKey;
            _data->key = attrsIt->second;
            _data->data.attrs.erase(attrsIt);
        }

        if (_data->sel[0] == '!') {
            _data->hash |= isComment;
            _data->sel = "";
        } else {
            std::erase(_data->children, nullptr);

            Attrs::iterator it = _data->data.attrs.begin();
            while (it != _data->data.attrs.end()) {
                if (it->first == "ns") {
                    _data->hash |= hasNS;
                    _data->ns = it->second;
                    it = _data->data.attrs.erase(it);
                } else if (it->second == "false") {
                    it = _data->data.attrs.erase(it);
                } else {
                    if (it->second == "true") {
                        it->second = "";
                    }
                    ++it;
                }
            }

            const bool addNS = injectSvgNamespace || (_data->sel == "svg");
            if (addNS) {
                _data->hash |= hasNS;
                _data->ns = "http://www.w3.org/2000/svg";
            }

            if (!_data->data.attrs.empty()) {
                _data->hash |= hasAttrs;
            }
            if (!_data->data.props.empty()) {
                _data->hash |= hasProps;
            }
            if (!_data->data.callbacks.empty()) {
                _data->hash |= hasCallbacks;
            }
            if (!_data->data.eventCallbacks.empty()) {
                _data->hash |= hasEventCallbacks;
            }
            if (!_data->children.empty()) {
                _data->hash |= hasDirectChildren;

                for (VNode& child : _data->children) {
                    child.normalize(addNS && _data->sel != "foreignObject");
                }
            }

            if (_data->sel.empty()) {
                _data->hash |= isFragment;
            } else {
                static std::size_t currentHash = 0;
                static std::unordered_map<std::string, std::size_t> hashes;

                if (!hashes.contains(_data->sel)) {
                    hashes.emplace(_data->sel, ++currentHash);
                }

                _data->hash |= (hashes[_data->sel] << 13) | isElement;
            }
        }

        _data->hash |= isNormalized;
    }
}

inline wasmdom::VNode wasmdom::VNode::toVNode(const emscripten::val& node)
{
    VNode vnode = nullptr;
    const int nodeType = node["nodeType"].as<int>();
    // isElement
    if (nodeType == 1) {
        std::string sel = node["tagName"].as<std::string>();
        internals::lower(sel);

        VNodeAttributes data;
        for (int i : std::views::iota(0, node["attributes"]["length"].as<int>())) {
            data.attrs.emplace(node["attributes"][i]["nodeName"].as<std::string>(), node["attributes"][i]["nodeValue"].as<std::string>());
        }

        Children children;
        for (int i : std::views::iota(0, node["childNodes"]["length"].as<int>())) {
            children.push_back(toVNode(node["childNodes"][i]));
        }

        vnode = VNode(sel, data)(children);
        // isText
    } else if (nodeType == 3) {
        vnode = VNode(text_tag, node["textContent"].as<std::string>());
        // isComment
    } else if (nodeType == 8) {
        vnode = VNode("!")(node["textContent"].as<std::string>());
    } else {
        vnode = VNode("");
    }
    vnode.setNode(node);
    return vnode;
}

inline std::string wasmdom::VNode::toHTML() const
{
    VNode vnode = *this;

    if (vnode)
        vnode.normalize();

    std::string html;
    internals::toHTML(vnode, html);
    return html;
}

inline void wasmdom::VNode::diff(const VNode& oldVnode)
{
    if (!*this || !oldVnode || *this == oldVnode)
        return;

    const std::size_t vnodes = _data->hash | oldVnode._data->hash;

    if (vnodes & hasAttrs) {
        internals::diffAttrs(oldVnode, *this);
    }
    if (vnodes & hasProps) {
        internals::diffProps(oldVnode, *this);
    }
    if (vnodes & hasCallbacks) {
        internals::diffCallbacks(oldVnode, *this);
    }
}

// End of single header library
