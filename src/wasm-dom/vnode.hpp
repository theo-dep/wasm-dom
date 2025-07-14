#pragma once

#include "attribute.hpp"
#include "vnodeforward.hpp"

namespace wasmdom
{

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

    struct VNode
    {
    private:
        void normalize(const bool injectSvgNamespace);

    public:
        VNode(std::string nodeSel)
            : sel{ nodeSel }
        {
        }
        VNode(std::string nodeSel,
              std::string nodeText)
            : sel{ nodeSel }
        {
            normalize();
            if (hash & isComment) {
                sel = nodeText;
            } else {
                children.push_back(new VNode(nodeText, true));
                hash |= hasText;
            }
        }
        VNode(std::string nodeText,
              bool textNode)
        {
            if (textNode) {
                normalize();
                sel = nodeText;
                // replace current type with text type
                hash = (hash & removeNodeType) | isText;
            } else {
                sel = nodeText;
                normalize();
            }
        }
        VNode(std::string nodeSel,
              Attributes nodeAttrs)
            : sel{ nodeSel }
            , data{ attributesToVNode(nodeAttrs) }
        {
        }
        VNode(std::string nodeSel,
              Children nodeChildren)
            : sel{ nodeSel }
            , children{ nodeChildren }
        {
        }
        VNode(std::string nodeSel,
              VNode* child)
            : sel{ nodeSel }
            , children{ child }
        {
        }
        VNode(std::string nodeSel,
              Attributes nodeAttrs,
              std::string nodeText)
            : sel{ nodeSel }
            , data{ attributesToVNode(nodeAttrs) }
        {
            normalize();
            if (hash & isComment) {
                sel = nodeText;
            } else {
                children.push_back(new VNode(nodeText, true));
                hash |= hasText;
            }
        }
        VNode(std::string nodeSel,
              Attributes nodeAttrs,
              Children nodeChildren)
            : sel{ nodeSel }
            , data{ attributesToVNode(nodeAttrs) }
            , children{ nodeChildren }
        {
        }
        VNode(std::string nodeSel,
              Attributes nodeAttrs,
              VNode* child)
            : sel{ nodeSel }
            , data{ attributesToVNode(nodeAttrs) }
            , children{ child }
        {
        }
        ~VNode();

        void normalize() { normalize(false); }

        // contains selector for elements and fragments, text for comments and textNodes
        std::string sel;
        std::string key;
        std::string ns;
        unsigned int hash = 0;
        VNodeAttributes data;
        int elm = 0;
        Children children;
    };

    void deleteVNode(const VNode* const vnode);

}
