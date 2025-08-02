#pragma once

#include <emscripten/val.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace wasmdom
{

    using Callback = std::function<bool(emscripten::val)>;
    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;

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

    struct Data
    {
        Data() {}
        Data(const Attrs& dataAttrs,
             const Props& dataProps = Props(),
             const Callbacks& dataCallbacks = Callbacks())
            : attrs(dataAttrs)
            , props(dataProps)
            , callbacks(dataCallbacks)
        {
        }
        Data(const Attrs& dataAttrs,
             const Callbacks& dataCallbacks)
            : attrs(dataAttrs)
            , callbacks(dataCallbacks)
        {
        }
        Data(const Props& dataProps,
             const Callbacks& dataCallbacks = Callbacks())
            : props(dataProps)
            , callbacks(dataCallbacks)
        {
        }
        Data(const Callbacks& dataCallbacks)
            : callbacks(dataCallbacks)
        {
        }
        ~Data();

        Attrs attrs;
        Props props;
        Callbacks callbacks;
    };

    class VNode
    {
        struct SharedData
        {
            std::string sel;
            std::string key;
            std::string ns;
            unsigned int hash = 0;
            Data data;
            int elm = 0;
            Children children;
        };

    public:
        VNode(std::nullptr_t);
        VNode(const std::string& nodeSel);
        VNode(const std::string& nodeSel,
              const std::string& nodeText);
        VNode(const std::string& nodeText,
              bool textNode);
        VNode(const std::string& nodeSel,
              const Data& nodeData);
        VNode(const std::string& nodeSel,
              const Children& nodeChildren);
        VNode(const std::string& nodeSel,
              const VNode& child);
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const std::string& nodeText);
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const Children& nodeChildren);
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const VNode& child);

        VNode(const VNode& other);
        VNode(VNode&& other);
        VNode& operator=(const VNode& other);
        VNode& operator=(VNode&& other);
        ~VNode();

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

#ifndef WASMDOM_COVERAGE
#include "vnode.inl.cpp"
#endif
