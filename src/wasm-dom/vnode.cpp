#include "vnode.hpp"

#include "domapi.hpp"
#include "domkeys.hpp"
#include "internals/diff.hpp"
#include "internals/tohtml.hpp"

#include <emscripten/bind.h>

#ifdef WASMDOM_COVERAGE
#include "vnode.inl.cpp"

wasmdom::Attributes::Attributes() = default;
wasmdom::Attributes::Attributes(const Attributes& other) = default;
wasmdom::Attributes::Attributes(Attributes&& other) = default;
wasmdom::Attributes& wasmdom::Attributes::operator=(const Attributes& other) = default;
wasmdom::Attributes& wasmdom::Attributes::operator=(Attributes&& other) = default;
wasmdom::Attributes::~Attributes() = default;

wasmdom::VNode::VNode(const VNode& other) = default;
wasmdom::VNode::VNode(VNode&& other) = default;
wasmdom::VNode& wasmdom::VNode::operator=(const VNode& other) = default;
wasmdom::VNode& wasmdom::VNode::operator=(VNode&& other) = default;
wasmdom::VNode::~VNode() = default;
#endif

void wasmdom::VNode::normalize(bool injectSvgNamespace)
{
    if (!_data)
        return;

    if (!(_data->hash & Flag::isNormalized)) {
        const auto attrsIt = _data->data.attrs.find("key");
        if (attrsIt != _data->data.attrs.cend()) {
            _data->hash |= Flag::hasKey;
            _data->key = attrsIt->second;
            _data->data.attrs.erase(attrsIt);
        }

        if (_data->sel[0] == '!') {
            _data->hash |= Flag::isComment;
            _data->sel = "";
        } else {
            std::erase(_data->children, nullptr);

            Attrs::iterator it = _data->data.attrs.begin();
            while (it != _data->data.attrs.end()) {
                if (it->first == "ns") {
                    _data->hash |= Flag::hasNS;
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

            const bool addNS = injectSvgNamespace || (_data->sel[0] == 's' && _data->sel[1] == 'v' && _data->sel[2] == 'g');
            if (addNS) {
                _data->hash |= Flag::hasNS;
                _data->ns = "http://www.w3.org/2000/svg";
            }

            if (!_data->data.attrs.empty())
                _data->hash |= Flag::hasAttrs;
            if (!_data->data.props.empty())
                _data->hash |= Flag::hasProps;
            if (!_data->data.callbacks.empty())
                _data->hash |= Flag::hasCallbacks;
            if (!_data->children.empty()) {
                _data->hash |= Flag::hasDirectChildren;

                for (VNode& child : _data->children) {
                    child.normalize(addNS && _data->sel != "foreignObject");
                }
            }

            if (_data->sel[0] == '\0') {
                _data->hash |= Flag::isFragment;
            } else {
                static std::size_t currentHash = 0;
                static std::unordered_map<std::string, std::size_t> hashes;

                if (hashes[_data->sel] == 0) {
                    hashes[_data->sel] = ++currentHash;
                }

                _data->hash |= (hashes[_data->sel] << Flag::MAX) | Flag::isElement;

                if ((_data->hash & Flag::hasCallbacks) && _data->data.callbacks.contains("ref")) {
                    _data->hash |= Flag::hasRef;
                }
            }
        }

        _data->hash |= Flag::isNormalized;
    }
}

wasmdom::VNode wasmdom::VNode::toVNode(const emscripten::val& node)
{
    VNode vnode = nullptr;
    const int nodeType = node["nodeType"].as<int>();
    // isElement
    if (nodeType == 1) {
        std::string sel = node["tagName"].as<std::string>();
        internals::lower(sel);

        Attributes data;
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

std::string wasmdom::VNode::toHTML() const
{
    VNode vnode = *this;

    if (vnode)
        vnode.normalize();

    std::string html;
    internals::toHTML(vnode, html);
    return html;
}

void wasmdom::VNode::diff(const VNode& oldVnode)
{
    if (!*this || !oldVnode || *this == oldVnode)
        return;

    const std::size_t vnodes = _data->hash | oldVnode._data->hash;

    if (vnodes & Flag::hasAttrs)
        internals::diffAttrs(oldVnode, *this);
    if (vnodes & Flag::hasProps)
        internals::diffProps(oldVnode, *this);
    if (vnodes & Flag::hasCallbacks)
        internals::diffCallbacks(oldVnode, *this);
}

namespace wasmdom::internals
{
    inline bool eventProxy(emscripten::val event)
    {
        const std::size_t callbackKey = event["currentTarget"][nodeCallbacksKey].as<std::size_t>();
        const std::string eventType = event["type"].as<std::string>();

        const Callbacks& callbacks = vnodeCallbacks()[callbackKey];
        auto callbackIt = callbacks.find(eventType);
        if (callbackIt == callbacks.cend()) {
            callbackIt = callbacks.find("on" + eventType);
        }
        return callbackIt->second(event);
    }
}

EMSCRIPTEN_BINDINGS(wasmdomEventModule)
{
    emscripten::function("eventProxy", wasmdom::internals::eventProxy);
}
