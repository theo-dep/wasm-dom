#include "internals/tohtml.hpp"

#include <wasm-dom/conf.h>
#include <wasm-dom/vnode.hpp>

#ifdef WASMDOM_COVERAGE
#include <wasm-dom/vnode.inl.hpp>

wasmdom::VNode::VNode(const VNode& other) = default;
wasmdom::VNode::VNode(VNode&& other) = default;
wasmdom::VNode& wasmdom::VNode::operator=(const VNode& other) = default;
wasmdom::VNode& wasmdom::VNode::operator=(VNode&& other) = default;
wasmdom::VNode::~VNode() = default;
#endif

WASMDOM_SH_INLINE
void wasmdom::VNode::normalize(bool injectSvgNamespace)
{
    if (!_data)
        return;

    if (!(_data->hash & isNormalized)) {
        const auto attrsIt = _data->attrs.find("key");
        if (attrsIt != _data->attrs.cend()) {
            _data->hash |= hasKey;
            _data->key = attrsIt->second;
            _data->attrs.erase(attrsIt);
        }

        if (_data->sel[0] == '!') {
            _data->hash |= isComment;
            _data->sel = "";
        } else {
            Attrs::iterator it = _data->attrs.begin();
            while (it != _data->attrs.end()) {
                if (it->first == "ns") {
                    _data->hash |= hasNS;
                    _data->ns = it->second;
                    it = _data->attrs.erase(it);
                } else if (it->second == "false") {
                    it = _data->attrs.erase(it);
                } else {
                    if (it->second == "true") {
                        it->second = "";
                    }
                    ++it;
                }
            }

            const bool addNS{ injectSvgNamespace || (_data->sel == "svg") };
            if (addNS) {
                _data->hash |= hasNS;
                _data->ns = "http://www.w3.org/2000/svg";
            }

            if (!_data->attrs.empty()) {
                _data->hash |= hasAttrs;
            }
#ifdef __EMSCRIPTEN__
            if (!_data->props.empty()) {
                _data->hash |= hasProps;
            }
            if (!_data->callbacks.empty()) {
                _data->hash |= hasCallbacks;
            }
            if (!_data->eventCallbacks.empty()) {
                _data->hash |= hasEventCallbacks;
            }
#endif

            if (!_children.empty()) {
                _data->hash |= hasDirectChildren;
                for (VNode& child : _children) {
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

                _data->hash |= (hashes[_data->sel] << maxFlags) | isElement;
            }
        }

        _data->hash |= isNormalized;
    }
}

WASMDOM_SH_INLINE
std::string wasmdom::VNode::toHTML() const
{
    VNode vnode = *this;
    vnode.normalize();

    std::string html;
    internals::toHTML(vnode, html);
    return html;
}

#ifdef __EMSCRIPTEN__

WASMDOM_SH_INLINE
wasmdom::VNode wasmdom::VNode::toVNode(const emscripten::val& node)
{
    if (node.isNull()) {
        return nullptr;
    }

    VNode vnode(nullptr);

    const int nodeType = node["nodeType"].as<int>();
    switch (nodeType) {
        case 1: // isElement
        {
            const std::string sel{ internals::lower(node["tagName"].as<std::string>()) };

            VNodeAttributes data;
            for (int i : std::views::iota(0, node["attributes"]["length"].as<int>())) {
                data.attrs.emplace(node["attributes"][i]["nodeName"].as<std::string>(), node["attributes"][i]["nodeValue"].as<std::string>());
            }

            const int childNodesLength{ node["childNodes"]["length"].as<int>() };
            std::vector<VNode> children(childNodesLength, nullptr);
            for (int i : std::views::iota(0, childNodesLength)) {
                children[i] = toVNode(node["childNodes"][i]);
            }

            vnode = VNode(sel, data)(children);
        } break;

        case 3: // isText
            vnode = VNode(text_tag, node["textContent"].as<std::string>());
            break;

        case 8: // isComment
            vnode = VNode("!")(node["textContent"].as<std::string>());
            break;

        default: // isDocumentFragment
        {
            // if fragment is not added to the DOM yet
            const int childNodesLength{ node["childElementCount"].as<int>() };
            std::vector<VNode> children(childNodesLength, nullptr);
            for (int i : std::views::iota(0, childNodesLength)) {
                children[i] = toVNode(node["children"][i]);
            }

            vnode = VNode("")(children);
        }
    }

    vnode._data->node = node;
    return vnode;
}

#endif
