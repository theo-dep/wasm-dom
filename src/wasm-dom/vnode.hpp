#pragma once

#include "attribute.hpp"

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
        hasDirectChildren = 1 << 9,
        hasChildren = hasDirectChildren | hasText,
        hasRef = 1 << 10,
        hasNS = 1 << 11,
        isNormalized = 1 << 12,

        // masks
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
            unsigned int hash = 0;
            VNodeAttributes data;
            int elm = 0;
            Children children;
        };

    public:
        VNode(std::nullptr_t);
        VNode(const std::string& nodeSel);
        VNode(text_tag_t, const std::string& nodeText);
        template <Stringifiable... K, Attribute... V>
        VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData);
        VNode(const std::string& nodeSel, const VNodeAttributes& nodeData);

        VNode& operator()(const std::string& nodeText);

        VNode& operator()(const Children::value_type& child);
        VNode& operator()(const Children& nodeChildren);
        VNode& operator()(std::initializer_list<Children::value_type> nodeChildren);

#ifdef WASMDOM_COVERAGE
        VNode(const VNode& other);
        VNode(VNode&& other);
        VNode& operator=(const VNode& other);
        VNode& operator=(VNode&& other);
        ~VNode();
#endif

        const Attrs& attrs() const;
        const Props& props() const;
        const Callbacks& callbacks() const;

        const std::string& sel() const;
        const std::string& key() const;
        const std::string& ns() const;
        unsigned int hash() const;
        int elm() const;

        void setElm(int nodeElm);

        const Children& children() const;

        void normalize();

        operator bool() const;
        bool operator!() const;
        bool operator==(const VNode& other) const;

        std::string toHTML() const;

        void diff(const VNode& other) const;

        static VNode toVNode(const emscripten::val& node);

    private:
        void normalize(bool injectSvgNamespace);

        friend void diffCallbacks(const VNode& oldVnode, const VNode& vnode);
        friend emscripten::val functionCallback(const std::uintptr_t& sharedData, std::string callback, emscripten::val event);

        friend int createElm(VNode& vnode);
        friend void patchVNode(VNode& oldVnode, VNode& vnode, int parentElm);

        // contains selector for elements and fragments, text for comments and textNodes
        std::shared_ptr<SharedData> _data = nullptr;
    };
}

template <wasmdom::Stringifiable... K, wasmdom::Attribute... V>
inline wasmdom::VNode::VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData)
    : VNode(nodeSel)
{
    _data->data = attributesToVNode(std::forward<std::pair<K, V>>(nodeData)...);
}

#ifndef WASMDOM_COVERAGE
#include "vnode.inl.cpp"
#endif
