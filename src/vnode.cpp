#include "internals/diff.hpp"
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

WASMDOM_SH_INLINE
wasmdom::VNode wasmdom::VNode::toVNode(const emscripten::val& node)
{
    VNode vnode = nullptr;

    const int nodeType = node["nodeType"].as<int>();
    switch (nodeType) {
        case 1: // isElement
        {
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
            Children children;
            for (int i : std::views::iota(0, node["childElementCount"].as<int>())) {
                children.push_back(toVNode(node["children"][i]));
            }

            vnode = VNode("")(children);
        }
    }

    vnode.setNode(node);
    vnode.setParentNode(internals::domapi::parentNode(node));

    return vnode;
}

WASMDOM_SH_INLINE
std::string wasmdom::VNode::toHTML() const
{
    VNode vnode = *this;

    if (vnode)
        vnode.normalize();

    std::string html;
    internals::toHTML(vnode, html);
    return html;
}

WASMDOM_SH_INLINE
void wasmdom::VNode::diff(const VNode& oldVnode)
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
