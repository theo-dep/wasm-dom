#pragma once

#include "wasm-dom/attribute.hpp"

#include <memory>
#include <vector>

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

        VNode(const VNode& other);
        VNode(VNode&& other);
        VNode& operator=(const VNode& other);
        VNode& operator=(VNode&& other);
        ~VNode();

        const Attrs& attrs() const;
#ifdef __EMSCRIPTEN__
        const Props& props() const;
        const Callbacks& callbacks() const;
        const EventCallbacks& eventCallbacks() const;
#endif

        const std::string& sel() const;
        const std::string& key() const;
        const std::string& ns() const;
        std::size_t hash() const;

#ifdef __EMSCRIPTEN__
        void setNode(const emscripten::val& node);
        const emscripten::val& node() const;
        emscripten::val& node();
#endif

        void updateParent(const VNode& oldVnode);
        const VNode& parent() const;

        void normalize();

        operator bool() const;
        bool operator!() const;
        bool operator==(const VNode& other) const;

        std::string toHTML() const;

#ifdef __EMSCRIPTEN__
        void diff(const VNode& other);

        static VNode toVNode(const emscripten::val& node);
#endif

        Children::iterator begin();
        Children::iterator end();
        Children::const_iterator begin() const;
        Children::const_iterator end() const;

    private:
        void normalize(bool injectSvgNamespace);

        // contains selector for elements and fragments, text for comments and textNodes
        struct SharedData;
        std::shared_ptr<SharedData> _data = nullptr;
    };

    struct VNode::SharedData
    {
        std::string sel;
        std::string key;
        std::string ns;
        std::size_t hash{ 0 };
        VNodeAttributes data;
#ifdef __EMSCRIPTEN__
        emscripten::val node{ emscripten::val::null() };
#endif
        const VNode* parent{ nullptr };
        Children children;
    };
}

template <wasmdom::AttributeKey... K, wasmdom::AttributeValue... V>
inline wasmdom::VNode::VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData)
    : VNode(nodeSel)
{
    _data->data = attributesToVNode(std::forward<std::pair<K, V>>(nodeData)...);
}

#ifndef WASMDOM_COVERAGE
#include "vnode.inl.hpp"
#endif
