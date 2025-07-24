#pragma once

#include "vnodeforward.hpp"

#include <emscripten/val.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace wasmdom
{

    using Callback = std::function<bool(emscripten::val)>;
    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;

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
    public:
        VNode(const std::string& nodeSel)
            : _sel(nodeSel)
        {
        }
        VNode(const std::string& nodeSel,
              const std::string& nodeText)
            : _sel(nodeSel)
        {
            normalize();
            if (_hash & isComment) {
                _sel = nodeText;
            } else {
                _children.push_back(new VNode(nodeText, true));
                _hash |= hasText;
            }
        }
        VNode(const std::string& nodeText,
              bool textNode)
        {
            if (textNode) {
                normalize();
                _sel = nodeText;
                // replace current type with text type
                _hash = (_hash & removeNodeType) | isText;
            } else {
                _sel = nodeText;
                normalize();
            }
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData)
            : _sel(nodeSel)
            , _data(nodeData)
        {
        }
        VNode(const std::string& nodeSel,
              const Children& nodeChildren)
            : _sel(nodeSel)
            , _children(nodeChildren)
        {
        }
        VNode(const std::string& nodeSel,
              VNode* child)
            : _sel(nodeSel)
            , _children{ child }
        {
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const std::string& nodeText)
            : _sel(nodeSel)
            , _data(nodeData)
        {
            normalize();
            if (_hash & isComment) {
                _sel = nodeText;
            } else {
                _children.push_back(new VNode(nodeText, true));
                _hash |= hasText;
            }
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              const Children& nodeChildren)
            : _sel(nodeSel)
            , _data(nodeData)
            , _children(nodeChildren)
        {
        }
        VNode(const std::string& nodeSel,
              const Data& nodeData,
              VNode* child)
            : _sel(nodeSel)
            , _data(nodeData)
            , _children{ child }
        {
        }
        ~VNode();

        const Attrs& attrs() const { return _data.attrs; }
        const Props& props() const { return _data.props; }
        const Callbacks& callbacks() const { return _data.callbacks; }

        const std::string& sel() const { return _sel; }
        const std::string& key() const { return _key; }
        const std::string& ns() const { return _ns; }
        unsigned int hash() const { return _hash; }
        int elm() const { return _elm; }

        void setElm(int nodeElm) { _elm = nodeElm; }

        const Children& children() const { return _children; }

        void normalize() { normalize(false); }

        std::string toHTML();

        void diff(const VNode* other) const;

        static VNode* toVNode(const emscripten::val& node);

    private:
        void normalize(bool injectSvgNamespace);

        // contains selector for elements and fragments, text for comments and textNodes
        std::string _sel;
        std::string _key;
        std::string _ns;
        unsigned int _hash = 0;
        Data _data;
        int _elm = 0;
        Children _children;
    };

}
