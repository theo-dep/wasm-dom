#include "vnode.hpp"

#include "domapi.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

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

    std::unordered_map<std::string, bool> containerElements{
        // http://www.w3.org/TR/SVG/intro.html#TermContainerElement
        { "a", true },
        { "defs", true },
        { "glyph", true },
        { "g", true },
        { "marker", true },
        { "mask", true },
        { "missing-glyph", true },
        { "pattern", true },
        { "svg", true },
        { "switch", true },
        { "symbol", true },
        { "text", true },

        // http://www.w3.org/TR/SVG/intro.html#TermDescriptiveElement
        { "desc", true },
        { "metadata", true },
        { "title", true }
    };

    // http://www.w3.org/html/wg/drafts/html/master/syntax.html#void-elements
    std::unordered_map<std::string, bool> voidElements{
        { "area", true },
        { "base", true },
        { "br", true },
        { "col", true },
        { "embed", true },
        { "hr", true },
        { "img", true },
        { "input", true },
        //{ "keygen", true },
        { "link", true },
        { "meta", true },
        { "param", true },
        { "source", true },
        { "track", true },
        { "wbr", true }
    };

    // https://developer.mozilla.org/en-US/docs/Web/API/element
    std::unordered_map<std::string, bool> omitProps{
        { "attributes", true },
        { "childElementCount", true },
        { "children", true },
        { "classList", true },
        { "clientHeight", true },
        { "clientLeft", true },
        { "clientTop", true },
        { "clientWidth", true },
        { "currentStyle", true },
        { "firstElementChild", true },
        { "innerHTML", true },
        { "lastElementChild", true },
        { "nextElementSibling", true },
        { "ongotpointercapture", true },
        { "onlostpointercapture", true },
        { "onwheel", true },
        { "outerHTML", true },
        { "previousElementSibling", true },
        { "runtimeStyle", true },
        { "scrollHeight", true },
        { "scrollLeft", true },
        { "scrollLeftMax", true },
        { "scrollTop", true },
        { "scrollTopMax", true },
        { "scrollWidth", true },
        { "tabStop", true },
        { "tagName", true }
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
            if (!omitProps[key]) {
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
            bool isSvgContainerElement = isSvg && containerElements[vnode.sel()];

            html.append("<" + vnode.sel());
            appendAttributes(vnode, html);
            if (isSvg && !isSvgContainerElement) {
                html.append(" /");
            }
            html.append(">");

            if (isSvgContainerElement ||
                (!isSvg && !voidElements[vnode.sel()])) {

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

        emscripten::val elm = emscripten::val::module_property("nodes")[vnode.elm()];

        EM_ASM({ Module['nodes'][$0]['asmDomRaws'] = []; }, vnode.elm());

        for (const auto& [key, _] : oldProps) {
            if (!props.contains(key)) {
                elm.set(key.c_str(), emscripten::val::undefined());
            }
        }

        for (const auto& [key, val] : props) {
            EM_ASM({ Module['nodes'][$0]['asmDomRaws'].push(Module['UTF8ToString']($1)); }, vnode.elm(), key.c_str());

            if (!oldProps.contains(key) ||
                !val.strictlyEquals(oldProps.at(key)) ||
                ((key == "value" || key == "checked") &&
                 !val.strictlyEquals(elm[key.c_str()]))) {
                elm.set(key.c_str(), val);
            }
        }
    }

    // store callbacks addresses to be called in functionCallback
    Callbacks* storeCallbacks(const VNode& oldVnode, const VNode& vnode)
    {
        static std::unordered_map<int, Callbacks> vnodeCallbacks;

        if (vnodeCallbacks.contains(oldVnode.elm())) {
            auto nodeHandle = vnodeCallbacks.extract(oldVnode.elm());
            nodeHandle.key() = vnode.elm();
            nodeHandle.mapped() = vnode.callbacks();
            vnodeCallbacks.insert(std::move(nodeHandle));
        } else {
            vnodeCallbacks.emplace(vnode.elm(), vnode.callbacks());
        }

        return &vnodeCallbacks[vnode.elm()];
    }

    void diffCallbacks(const VNode& oldVnode, const VNode& vnode)
    {
        const Callbacks& oldCallbacks = oldVnode.callbacks();
        const Callbacks& callbacks = vnode.callbacks();

        for (const auto& [key, _] : oldCallbacks) {
            if (!callbacks.contains(key) && key != "ref") {
                EM_ASM({
                    var key = Module['UTF8ToString']($1).replace(/^on/, "");
                    var elm = Module['nodes'][$0];
                    elm.removeEventListener(
                        key,
                        Module['eventProxy'],
                        false
                    );
                    delete elm['asmDomEvents'][key]; }, vnode.elm(), key.c_str());
            }
        }

        EM_ASM({
            var elm = Module['nodes'][$0];
            elm['asmDomVNodeCallbacks'] = $1;
            if (elm['asmDomEvents'] === undefined) {
                elm['asmDomEvents'] = {};
            } }, vnode.elm(), storeCallbacks(oldVnode, vnode));

        for (const auto& [key, _] : callbacks) {
            if (!oldCallbacks.contains(key) && key != "ref") {
                EM_ASM({
                    var key = Module['UTF8ToString']($1).replace(/^on/, "");
                    var elm = Module['nodes'][$0];
                    elm.addEventListener(
                        key,
                        Module['eventProxy'],
                        false
                    );
                    elm['asmDomEvents'][key] = Module['eventProxy']; }, vnode.elm(), key.c_str());
            }
        }

        if (vnode.hash() & hasRef) {
            bool (*const* callback)(emscripten::val) = callbacks.at("ref").target<bool (*)(emscripten::val)>();
            bool (*const* oldCallback)(emscripten::val) = oldVnode.hash() & hasRef ? oldCallbacks.at("ref").target<bool (*)(emscripten::val)>() : nullptr;
            if (!callback || !oldCallback || *oldCallback != *callback) {
                if (oldVnode.hash() & hasRef) {
                    oldCallbacks.at("ref")(emscripten::val::null());
                }
                callbacks.at("ref")(emscripten::val::module_property("nodes")[vnode.elm()]);
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

    emscripten::val functionCallback(const std::uintptr_t& sharedData, std::string callback, emscripten::val event)
    {
        Callbacks* cbs = reinterpret_cast<Callbacks*>(sharedData);
        if (!cbs->contains(callback)) {
            callback = "on" + callback;
        }
        return emscripten::val((*cbs)[callback](event));
    }

}

EMSCRIPTEN_BINDINGS(functionCallback)
{
    emscripten::function("functionCallback", &wasmdom::functionCallback, emscripten::allow_raw_pointers());
}
