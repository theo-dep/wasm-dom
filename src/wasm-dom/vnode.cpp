#include "vnode.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

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

        Data data;
        int i = node["attributes"]["length"].as<int>();
        while (i--) {
            data.attrs.emplace(node["attributes"][i]["nodeName"].as<std::string>(), node["attributes"][i]["nodeValue"].as<std::string>());
        }

        Children children;
        i = 0;
        for (int n = node["childNodes"]["length"].as<int>(); i < n; ++i) {
            children.push_back(toVNode(node["childNodes"][i]));
        }

        vnode = VNode(sel, data, children);
        // isText
    } else if (nodeType == 3) {
        vnode = VNode(node["textContent"].as<std::string>(), true);
        // isComment
    } else if (nodeType == 8) {
        vnode = VNode("!", node["textContent"].as<std::string>());
    } else {
        vnode = VNode("");
    }
    vnode._data->elm = emscripten::val::module_property("addNode")(node).as<int>();
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
        { "keygen", true },
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

        for (const auto& it : oldAttrs) {
            if (!attrs.contains(it.first)) {
                EM_ASM_({ Module.removeAttribute(
                              $0,
                              Module['UTF8ToString']($1)); }, vnode.elm(), it.first.c_str());
            }
        }

        for (const auto& it : attrs) {
            if (!oldAttrs.contains(it.first) || oldAttrs.at(it.first) != it.second) {
                EM_ASM_({ Module.setAttribute(
                              $0,
                              Module['UTF8ToString']($1),
                              Module['UTF8ToString']($2)); }, vnode.elm(), it.first.c_str(), it.second.c_str());
            }
        }
    }

    void diffProps(const VNode& oldVnode, const VNode& vnode)
    {
        const Props& oldProps = oldVnode.props();
        const Props& props = vnode.props();

        emscripten::val elm = emscripten::val::module_property("nodes")[vnode.elm()];

        EM_ASM_({ Module['nodes'][$0]['asmDomRaws'] = []; }, vnode.elm());

        for (const auto& it : oldProps) {
            if (!props.contains(it.first)) {
                elm.set(it.first.c_str(), emscripten::val::undefined());
            }
        }

        for (const auto& it : props) {
            EM_ASM_({ Module['nodes'][$0]['asmDomRaws'].push(Module['UTF8ToString']($1)); }, vnode.elm(), it.first.c_str());

            if (
                !oldProps.contains(it.first) ||
                !it.second.strictlyEquals(oldProps.at(it.first)) ||
                ((it.first == "value" || it.first == "checked") &&
                 !it.second.strictlyEquals(elm[it.first.c_str()]))) {
                elm.set(it.first.c_str(), it.second);
            }
        }
    }

    void diffCallbacks(const VNode& oldVnode, const VNode& vnode)
    {
        const Callbacks& oldCallbacks = oldVnode.callbacks();
        const Callbacks& callbacks = vnode.callbacks();

        for (const auto& it : oldCallbacks) {
            if (!callbacks.contains(it.first) && it.first != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.removeEventListener(
						key,
						Module['eventProxy'],
						false
					);
					delete elm['asmDomEvents'][key]; }, vnode.elm(), it.first.c_str());
            }
        }

        EM_ASM_({
			var elm = Module['nodes'][$0];
			elm['asmDomVNode'] = $1;
			if (elm['asmDomEvents'] === undefined) {
				elm['asmDomEvents'] = {};
			} }, vnode.elm(), reinterpret_cast<std::uintptr_t>(vnode._data.get()));

        for (const auto& it : callbacks) {
            if (!oldCallbacks.contains(it.first) && it.first != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.addEventListener(
						key,
						Module['eventProxy'],
						false
					);
					elm['asmDomEvents'][key] = Module['eventProxy']; }, vnode.elm(), it.first.c_str());
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
        const Callbacks& cbs = reinterpret_cast<VNode::SharedData*>(sharedData)->data.callbacks;
        if (!cbs.contains(callback)) {
            callback = "on" + callback;
        }
        return emscripten::val(cbs.at(callback)(event));
    }

}

EMSCRIPTEN_BINDINGS(functionCallback)
{
    emscripten::function("functionCallback", &wasmdom::functionCallback, emscripten::allow_raw_pointers());
}
