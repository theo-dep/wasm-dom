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
        VNode(std::nullptr_t) {}
        VNode(const std::string& nodeSel)
            : _data(std::make_shared<SharedData>())
        {
            _data->sel = nodeSel;
        }
        VNode(const std::string& nodeSel,
              const std::string& nodeText)
            : VNode(nodeSel)
        {
            normalize();
            if (_data->hash & isComment) {
                _data->sel = nodeText;
            } else {
                _data->children.emplace_back(nodeText, true);
                _data->hash |= hasText;
            }
        }
        VNode(const std::string& nodeText,
              bool textNode)
            : _data(std::make_shared<SharedData>())
        {
            if (textNode) {
                normalize();
                _data->sel = nodeText;
                // replace current type with text type
                _data->hash = (_data->hash & removeNodeType) | isText;
            } else {
                _data->sel = nodeText;
                normalize();
            }
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData)
            : VNode(nodeSel)
        {
            _data->data = nodeData;
        }
        VNode(const std::string& nodeSel,
              const Children& nodeChildren)
            : VNode(nodeSel)
        {
            _data->children = nodeChildren;
        }
        VNode(const std::string& nodeSel,
              const VNode& child)
            : VNode(nodeSel)
        {
            _data->children.push_back(child);
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const std::string& nodeText)
            : VNode(nodeSel, nodeData)
        {
            normalize();
            if (_data->hash & isComment) {
                _data->sel = nodeText;
            } else {
                _data->children.emplace_back(nodeText, true);
                _data->hash |= hasText;
            }
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const Children& nodeChildren)
            : VNode(nodeSel, nodeData)
        {
            _data->children = nodeChildren;
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const VNode& child)
            : VNode(nodeSel, nodeData)
        {
            _data->children.push_back(child);
        }

        const Attrs& attrs() const { return _data->data.attrs; }
        const Props& props() const { return _data->data.props; }
        const Callbacks& callbacks() const { return _data->data.callbacks; }

        const std::string& sel() const { return _data->sel; }
        const std::string& key() const { return _data->key; }
        const std::string& ns() const { return _data->ns; }
        unsigned int hash() const { return _data->hash; }
        int elm() const { return _data->elm; }

        void setElm(int nodeElm) { _data->elm = nodeElm; }

        const Children& children() const { return _data->children; }

        void normalize() { normalize(false); }

        std::string toHTML() const;

        void diff(const VNode& other) const;

        operator bool() const { return _data != nullptr; }
        bool operator!() const { return _data == nullptr; }
        std::strong_ordering operator<=>(const VNode& other) const = default;

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
