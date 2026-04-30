#pragma once

#include "wasm-dom/attribute.hpp"

#include <cstdint>
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

#ifdef __EMSCRIPTEN__
    namespace internals
    {
        // Index into the JS-side handle table (`Module.__wdomTable`).
        // 0 is the null sentinel; non-zero ids each hold one refcount.
        using NodeId = std::uint32_t;
        inline constexpr NodeId nullNodeId = 0;

        // JS-side handle table primitives. allocNode takes a val and
        // returns a freshly retained id (0 for null/undefined).
        // resolveNode produces a new val handle for an id (id is unaffected).
        // retain/drop adjust the refcount; the slot is freed when it hits 0.
        NodeId allocNode(const emscripten::val& v);
        emscripten::val resolveNode(NodeId id);
        void retainNode(NodeId id);
        void dropNode(NodeId id);
    }
#endif

    class VNode
    {
        struct SharedData
        {
            std::string sel;
            std::string key;
            std::string ns;
            std::size_t hash{ 0 };
            VNodeAttributes data;
#ifdef __EMSCRIPTEN__
            // Indices into the JS-side handle table (0 == null sentinel).
            // Each non-zero id holds one refcount; released by ~SharedData.
            internals::NodeId node{ internals::nullNodeId };
            internals::NodeId parentNode{ internals::nullNodeId };
            // Event listener wrappers actually attached to `node`, keyed by
            // the formatted event name (no "on" prefix). Mirrors the JS-side
            // wasmDomEvents map so the diff can avoid live DOM reads.
            std::unordered_map<std::string, emscripten::val> installedListeners;
            ~SharedData();
#endif
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

#ifdef WASMDOM_COVERAGE
        VNode(const VNode& other);
        VNode(VNode&& other);
        VNode& operator=(const VNode& other);
        VNode& operator=(VNode&& other);
        ~VNode();
#endif

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
        emscripten::val node() const;
        emscripten::val parentNode() const;

        // Internal accessors for the JS-side handle ids (no JS round-trip).
        internals::NodeId nodeId() const;
        internals::NodeId parentNodeId() const;

        void setNode(const emscripten::val& node);
        void setParentNode(const emscripten::val& node);

        // Take a retained reference on `id` (caller keeps its own ref).
        void setNodeId(internals::NodeId id);
        void setParentNodeId(internals::NodeId id);

        std::unordered_map<std::string, emscripten::val>& installedListeners();
        const std::unordered_map<std::string, emscripten::val>& installedListeners() const;
#endif

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
        std::shared_ptr<SharedData> _data = nullptr;
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
