#include "vnode.hpp"

#include "domapi.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

#include <array>
#include <regex>

#ifdef WASMDOM_COVERAGE
#include "vnode.inl.cpp"

wasmdom::VNodeAttributes::VNodeAttributes() = default;
wasmdom::VNodeAttributes::VNodeAttributes(const VNodeAttributes& other) = default;
wasmdom::VNodeAttributes::VNodeAttributes(VNodeAttributes&& other) = default;
wasmdom::VNodeAttributes& wasmdom::VNodeAttributes::operator=(const VNodeAttributes& other) = default;
wasmdom::VNodeAttributes& wasmdom::VNodeAttributes::operator=(VNodeAttributes&& other) = default;
wasmdom::VNodeAttributes::~VNodeAttributes() = default;

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

    if (!(_data->hash & isNormalized)) {
        if (_data->data.attrs.contains("key")) {
            _data->hash |= hasKey;
            _data->key = _data->data.attrs["key"];
            _data->data.attrs.erase("key");
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

            bool addNS = injectSvgNamespace || (_data->sel[0] == 's' && _data->sel[1] == 'v' && _data->sel[2] == 'g');
            if (addNS) {
                _data->hash |= hasNS;
                _data->ns = "http://www.w3.org/2000/svg";
            }

            if (!_data->data.attrs.empty())
                _data->hash |= hasAttrs;
            if (!_data->data.props.empty())
                _data->hash |= hasProps;
            if (!_data->data.callbacks.empty())
                _data->hash |= hasCallbacks;
            if (!_data->children.empty()) {
                _data->hash |= hasDirectChildren;

                Children::size_type i = _data->children.size();
                while (i--) {
                    _data->children[i].normalize(addNS && _data->sel != "foreignObject");
                }
            }

            if (_data->sel[0] == '\0') {
                _data->hash |= isFragment;
            } else {
                static unsigned int currentHash = 0;
                static std::unordered_map<std::string, unsigned int> hashes;

                if (hashes[_data->sel] == 0) {
                    hashes[_data->sel] = ++currentHash;
                }

                _data->hash |= (hashes[_data->sel] << 13) | isElement;

                if ((_data->hash & hasCallbacks) && _data->data.callbacks.contains("ref")) {
                    _data->hash |= hasRef;
                }
            }
        }

        _data->hash |= isNormalized;
    }
}

wasmdom::VNode wasmdom::VNode::toVNode(const emscripten::val& node)
{
    VNode vnode = nullptr;
    int nodeType = node["nodeType"].as<int>();
    // isElement
    if (nodeType == 1) {
        std::string sel = node["tagName"].as<std::string>();
        std::transform(sel.begin(), sel.end(), sel.begin(), ::tolower);

        VNodeAttributes data;
        int i = node["attributes"]["length"].as<int>();
        while (i--) {
            data.attrs.emplace(node["attributes"][i]["nodeName"].as<std::string>(), node["attributes"][i]["nodeValue"].as<std::string>());
        }

        Children children;
        i = 0;
        for (int n = node["childNodes"]["length"].as<int>(); i < n; ++i) {
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
    vnode._data->elm = domapi::addNode(node);
    return vnode;
}

namespace wasmdom
{

    // All SVG children elements, not in this list, should self-close

    static constexpr inline std::array containerElements{
        // http://www.w3.org/TR/SVG/intro.html#TermContainerElement
        "a",
        "defs",
        "glyph",
        "g",
        "marker",
        "mask",
        "missing-glyph",
        "pattern",
        "svg",
        "switch",
        "symbol",
        "text",

        // http://www.w3.org/TR/SVG/intro.html#TermDescriptiveElement
        "desc",
        "metadata",
        "title"
    };

    // http://www.w3.org/html/wg/drafts/html/master/syntax.html#void-elements
    static constexpr inline std::array voidElements{
        "area",
        "base",
        "br",
        "col",
        "embed",
        "hr",
        "img",
        "input",
        //"keygen",
        "link",
        "meta",
        "param",
        "source",
        "track",
        "wbr"
    };

    // https://developer.mozilla.org/en-US/docs/Web/API/element
    static constexpr inline std::array omitProps{
        "attributes",
        "childElementCount",
        "children",
        "classList",
        "clientHeight",
        "clientLeft",
        "clientTop",
        "clientWidth",
        "currentStyle",
        "firstElementChild",
        "innerHTML",
        "lastElementChild",
        "nextElementSibling",
        "ongotpointercapture",
        "onlostpointercapture",
        "onwheel",
        "outerHTML",
        "previousElementSibling",
        "runtimeStyle",
        "scrollHeight",
        "scrollLeft",
        "scrollLeftMax",
        "scrollTop",
        "scrollTopMax",
        "scrollWidth",
        "tabStop",
        "tagName"
    };

    std::string encode(const std::string& data)
    {
        std::string encoded;
        std::size_t size = data.size();
        encoded.reserve(size);
        for (std::size_t pos = 0; pos != size; ++pos) {
            switch (data[pos]) {
                case '&':
                    encoded.append("&amp;");
                    break;
                case '\"':
                    encoded.append("&quot;");
                    break;
                case '\'':
                    encoded.append("&apos;");
                    break;
                case '<':
                    encoded.append("&lt;");
                    break;
                case '>':
                    encoded.append("&gt;");
                    break;
                case '`':
                    encoded.append("&#96;");
                    break;
                default:
                    encoded.append(&data[pos], 1);
                    break;
            }
        }
        return encoded;
    }

    void appendAttributes(const VNode& vnode, std::string& html)
    {
        for (const auto& [key, val] : vnode.attrs()) {
            html.append(" " + key + "=\"" + encode(val) + "\"");
        }

        emscripten::val String = emscripten::val::global("String");
        for (const auto& [key, val] : vnode.props()) {
            if (std::find(omitProps.cbegin(), omitProps.cend(), key) == omitProps.cend()) {
                std::string lowerKey(key);
                std::transform(key.begin(), key.end(), lowerKey.begin(), ::tolower);
                html.append(" " + lowerKey + "=\"" + encode(String(val).as<std::string>()) + "\"");
            }
        }
    }

    void toHTML(const VNode& vnode, std::string& html)
    {
        if (!vnode)
            return;

        if (vnode.hash() & isText && !vnode.sel().empty()) {
            html.append(encode(vnode.sel()));
        } else if (vnode.hash() & isComment) {
            html.append("<!--" + vnode.sel() + "-->");
        } else if (vnode.hash() & isFragment) {
            for (const VNode& child : vnode.children()) {
                toHTML(child, html);
            }
        } else {
            bool isSvg = (vnode.hash() & hasNS) && vnode.ns() == "http://www.w3.org/2000/svg";
            bool isSvgContainerElement = isSvg &&
                                         std::find(containerElements.cbegin(), containerElements.cend(), vnode.sel()) != containerElements.cend();

            html.append("<" + vnode.sel());
            appendAttributes(vnode, html);
            if (isSvg && !isSvgContainerElement) {
                html.append(" /");
            }
            html.append(">");

            if (isSvgContainerElement ||
                (!isSvg && std::find(voidElements.cbegin(), voidElements.cend(), vnode.sel()) == voidElements.cend())) {

                if (vnode.props().contains("innerHTML") != 0) {
                    html.append(vnode.props().at("innerHTML").as<std::string>());
                } else {
                    for (const VNode& child : vnode.children()) {
                        toHTML(child, html);
                    }
                }
                html.append("</" + vnode.sel() + ">");
            }
        }
    }

}

std::string wasmdom::VNode::toHTML() const
{
    VNode vnode = *this;

    if (vnode)
        vnode.normalize();

    std::string html;
    wasmdom::toHTML(vnode, html);
    return html;
}

namespace wasmdom
{

    void diffAttrs(const VNode& oldVnode, const VNode& vnode)
    {
        const Attrs& oldAttrs = oldVnode.attrs();
        const Attrs& attrs = vnode.attrs();

        for (const auto& [key, _] : oldAttrs) {
            if (!attrs.contains(key)) {
                domapi::removeAttribute(vnode.elm(), key);
            }
        }

        for (const auto& [key, val] : attrs) {
            if (!oldAttrs.contains(key) || oldAttrs.at(key) != val) {
                domapi::setAttribute(vnode.elm(), key, val);
            }
        }
    }

    void diffProps(const VNode& oldVnode, const VNode& vnode)
    {
        const Props& oldProps = oldVnode.props();
        const Props& props = vnode.props();

        emscripten::val node = domapi::node(vnode.elm());
        node.set("asmDomRaws", emscripten::val::array());

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                node.set(key, emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            node["asmDomRaws"].call<void>("push", key);

            if (!oldProps.contains(key) ||
                !val.strictlyEquals(oldProps.at(key)) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(node[key]))) {
                node.set(key, val);
            }
        }
    }

    // store callbacks addresses to be called in functionCallback
    std::unordered_map<int, Callbacks>& vnodeCallbacks()
    {
        static std::unordered_map<int, Callbacks> vnodeCallbacks;
        return vnodeCallbacks;
    }
    emscripten::val storeCallbacks(const VNode& oldVnode, const VNode& vnode)
    {
        if (vnodeCallbacks().contains(oldVnode.elm())) {
            auto nodeHandle = vnodeCallbacks().extract(oldVnode.elm());
            nodeHandle.key() = vnode.elm();
            nodeHandle.mapped() = vnode.callbacks();
            vnodeCallbacks().insert(std::move(nodeHandle));
        } else {
            vnodeCallbacks().emplace(vnode.elm(), vnode.callbacks());
        }

        return emscripten::val(vnode.elm());
    }

    std::string formatEventKey(const std::string& key)
    {
        static const std::regex eventPrefix("^on");
        std::string eventKey;
        std::regex_replace(std::back_inserter(eventKey), key.cbegin(), key.cend(), eventPrefix, "");
        return eventKey;
    }

    void diffCallbacks(const VNode& oldVnode, const VNode& vnode)
    {
        const Callbacks& oldCallbacks = oldVnode.callbacks();
        const Callbacks& callbacks = vnode.callbacks();

        emscripten::val node = domapi::node(vnode.elm());

        std::string eventKey;

        for (const auto& [key, _] : oldCallbacks) {
            if (!callbacks.contains(key) && key != "ref") {
                eventKey = formatEventKey(key);
                node.call<void>("removeEventListener", eventKey, emscripten::val::module_property("eventProxy"), false);
                node["asmDomEvents"].delete_(eventKey);
            }
        }

        node.set("asmDomVNodeCallbacks", storeCallbacks(oldVnode, vnode));
        if (node["asmDomEvents"] == emscripten::val::undefined()) {
            node.set("asmDomEvents", emscripten::val::object());
        }

        for (const auto& [key, _] : callbacks) {
            if (!oldCallbacks.contains(key) && key != "ref") {
                eventKey = formatEventKey(key);
                node.call<void>("addEventListener", eventKey, emscripten::val::module_property("eventProxy"), false);
                node["asmDomEvents"].set(eventKey, emscripten::val::module_property("eventProxy"));
            }
        }

        if (vnode.hash() & hasRef) {
            bool (*const* callback)(emscripten::val) = callbacks.at("ref").target<bool (*)(emscripten::val)>();
            bool (*const* oldCallback)(emscripten::val) = oldVnode.hash() & hasRef ? oldCallbacks.at("ref").target<bool (*)(emscripten::val)>() : nullptr;
            if (!callback || !oldCallback || *oldCallback != *callback) {
                if (oldVnode.hash() & hasRef) {
                    oldCallbacks.at("ref")(emscripten::val::null());
                }
                callbacks.at("ref")(node);
            }
        } else if (oldVnode.hash() & hasRef) {
            oldCallbacks.at("ref")(emscripten::val::null());
        }
    }

}

void wasmdom::VNode::diff(const VNode& oldVnode) const
{
    if (!*this || !oldVnode)
        return;

    const unsigned int vnodes = _data->hash | oldVnode._data->hash;

    if (vnodes & hasAttrs)
        diffAttrs(oldVnode, *this);
    if (vnodes & hasProps)
        diffProps(oldVnode, *this);
    if (vnodes & hasCallbacks)
        diffCallbacks(oldVnode, *this);
}

namespace wasmdom
{

    emscripten::val functionCallback(int nodePtr, std::string callback, emscripten::val event)
    {
        const Callbacks& cbs = vnodeCallbacks()[nodePtr];
        if (!cbs.contains(callback)) {
            callback = "on" + callback;
        }
        return emscripten::val(cbs.at(callback)(event));
    }

}

EMSCRIPTEN_BINDINGS(functionCallback)
{
    emscripten::function("functionCallback", wasmdom::functionCallback);
}
