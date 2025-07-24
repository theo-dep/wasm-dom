#include "vnode.hpp"

#include "vnodeforward.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

namespace wasmdom
{

    unsigned int currentHash = 0;
    std::unordered_map<std::string, unsigned int> hashes;

}

void wasmdom::VNode::normalize(bool injectSvgNamespace)
{
    if (!(_hash & isNormalized)) {
        if (_data.attrs.count("key")) {
            _hash |= hasKey;
            _key = _data.attrs["key"];
            _data.attrs.erase("key");
        }

        if (_sel[0] == '!') {
            _hash |= isComment;
            _sel = "";
        } else {
            _children.erase(std::remove(_children.begin(), _children.end(), (VNode*)nullptr), _children.end());

            Attrs::iterator it = _data.attrs.begin();
            while (it != _data.attrs.end()) {
                if (it->first == "ns") {
                    _hash |= hasNS;
                    _ns = it->second;
                    it = _data.attrs.erase(it);
                } else if (it->second == "false") {
                    it = _data.attrs.erase(it);
                } else {
                    if (it->second == "true") {
                        it->second = "";
                    }
                    ++it;
                }
            }

            bool addNS = injectSvgNamespace || (_sel[0] == 's' && _sel[1] == 'v' && _sel[2] == 'g');
            if (addNS) {
                _hash |= hasNS;
                _ns = "http://www.w3.org/2000/svg";
            }

            if (!_data.attrs.empty())
                _hash |= hasAttrs;
            if (!_data.props.empty())
                _hash |= hasProps;
            if (!_data.callbacks.empty())
                _hash |= hasCallbacks;
            if (!_children.empty()) {
                _hash |= hasDirectChildren;

                Children::size_type i = _children.size();
                while (i--) {
                    _children[i]->normalize(addNS && _sel != "foreignObject");
                }
            }

            if (_sel[0] == '\0') {
                _hash |= isFragment;
            } else {
                if (hashes[_sel] == 0) {
                    hashes[_sel] = ++currentHash;
                }

                _hash |= (hashes[_sel] << 13) | isElement;

                if ((_hash & hasCallbacks) && _data.callbacks.count("ref")) {
                    _hash |= hasRef;
                }
            }
        }

        _hash |= isNormalized;
    }
}

void wasmdom::deleteVNode(const VNode* vnode)
{
    if (!(vnode->hash() & hasText)) {
        Children::size_type i = vnode->children().size();
        while (i--)
            deleteVNode(vnode->children()[i]);
    }
    delete vnode;
}

wasmdom::VNode::~VNode()
{
    if (_hash & hasText) {
        Children::size_type i = _children.size();
        while (i--)
            delete _children[i];
    }
}

wasmdom::VNode* wasmdom::VNode::toVNode(const emscripten::val& node)
{
    VNode* vnode;
    int nodeType = node["nodeType"].as<int>();
    // isElement
    if (nodeType == 1) {
        std::string sel = node["tagName"].as<std::string>();
        std::transform(sel.begin(), sel.end(), sel.begin(), ::tolower);

        Data data;
        int i = node["attributes"]["length"].as<int>();
        while (i--) {
            data.attrs.insert(
                std::make_pair(
                    node["attributes"][i]["nodeName"].as<std::string>(),
                    node["attributes"][i]["nodeValue"].as<std::string>()));
        }

        Children children;
        i = 0;
        for (int n = node["childNodes"]["length"].as<int>(); i < n; ++i) {
            children.push_back(toVNode(node["childNodes"][i]));
        }

        vnode = new VNode(sel, data, children);
        // isText
    } else if (nodeType == 3) {
        vnode = new VNode(node["textContent"].as<std::string>(), true);
        // isComment
    } else if (nodeType == 8) {
        vnode = new VNode("!", node["textContent"].as<std::string>());
    } else {
        vnode = new VNode("");
    }
    vnode->_elm = emscripten::val::module_property("addNode")(node).as<int>();
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

    void appendAttributes(const VNode* vnode, std::string& html)
    {
        for (const auto& [key, val] : vnode->attrs()) {
            html.append(" " + key + "=\"" + encode(val) + "\"");
        }

        emscripten::val String = emscripten::val::global("String");
        for (const auto& [key, val] : vnode->props()) {
            if (!omitProps[key]) {
                std::string lowerKey(key);
                std::transform(key.begin(), key.end(), lowerKey.begin(), ::tolower);
                html.append(" " + lowerKey + "=\"" + encode(String(val).as<std::string>()) + "\"");
            }
        }
    }

    void toHTML(const VNode* vnode, std::string& html)
    {
        if (!vnode)
            return;

        if (vnode->hash() & isText && !vnode->sel().empty()) {
            html.append(encode(vnode->sel()));
        } else if (vnode->hash() & isComment) {
            html.append("<!--" + vnode->sel() + "-->");
        } else if (vnode->hash() & isFragment) {
            for (const VNode* child : vnode->children()) {
                toHTML(child, html);
            }
        } else {
            bool isSvg = (vnode->hash() & hasNS) && vnode->ns() == "http://www.w3.org/2000/svg";
            bool isSvgContainerElement = isSvg && containerElements[vnode->sel()];

            html.append("<" + vnode->sel());
            appendAttributes(vnode, html);
            if (isSvg && !isSvgContainerElement) {
                html.append(" /");
            }
            html.append(">");

            if (isSvgContainerElement ||
                (!isSvg && !voidElements[vnode->sel()])) {

                if (vnode->props().count("innerHTML") != 0) {
                    html.append(vnode->props().at("innerHTML").as<std::string>());
                } else {
                    for (const VNode* child : vnode->children()) {
                        toHTML(child, html);
                    }
                }
                html.append("</" + vnode->sel() + ">");
            }
        }
    }

}

std::string wasmdom::VNode::toHTML()
{
    normalize();

    std::string html;
    wasmdom::toHTML(this, html);
    return html;
}

namespace wasmdom
{

    void diffAttrs(const VNode* oldVnode, const VNode* vnode)
    {
        const Attrs& oldAttrs = oldVnode->attrs();
        const Attrs& attrs = vnode->attrs();

        for (const auto& it : oldAttrs) {
            if (!attrs.count(it.first)) {
                EM_ASM_({ Module.removeAttribute(
                              $0,
                              Module['UTF8ToString']($1)); }, vnode->elm(), it.first.c_str());
            }
        }

        for (const auto& it : attrs) {
            if (!oldAttrs.count(it.first) || oldAttrs.at(it.first) != it.second) {
                EM_ASM_({ Module.setAttribute(
                              $0,
                              Module['UTF8ToString']($1),
                              Module['UTF8ToString']($2)); }, vnode->elm(), it.first.c_str(), it.second.c_str());
            }
        }
    }

    void diffProps(const VNode* oldVnode, const VNode* vnode)
    {
        const Props& oldProps = oldVnode->props();
        const Props& props = vnode->props();

        emscripten::val elm = emscripten::val::module_property("nodes")[vnode->elm()];

        EM_ASM_({ Module['nodes'][$0]['asmDomRaws'] = []; }, vnode->elm());

        for (const auto& it : oldProps) {
            if (!props.count(it.first)) {
                elm.set(it.first.c_str(), emscripten::val::undefined());
            }
        }

        for (const auto& it : props) {
            EM_ASM_({ Module['nodes'][$0]['asmDomRaws'].push(Module['UTF8ToString']($1)); }, vnode->elm(), it.first.c_str());

            if (
                !oldProps.count(it.first) ||
                !it.second.strictlyEquals(oldProps.at(it.first)) ||
                ((it.first == "value" || it.first == "checked") &&
                 !it.second.strictlyEquals(elm[it.first.c_str()]))) {
                elm.set(it.first.c_str(), it.second);
            }
        }
    }

    void diffCallbacks(const VNode* oldVnode, const VNode* vnode)
    {
        const Callbacks& oldCallbacks = oldVnode->callbacks();
        const Callbacks& callbacks = vnode->callbacks();

        for (const auto& it : oldCallbacks) {
            if (!callbacks.count(it.first) && it.first != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.removeEventListener(
						key,
						Module['eventProxy'],
						false
					);
					delete elm['asmDomEvents'][key]; }, vnode->elm(), it.first.c_str());
            }
        }

        EM_ASM_({
			var elm = Module['nodes'][$0];
			elm['asmDomVNode'] = $1;
			if (elm['asmDomEvents'] === undefined) {
				elm['asmDomEvents'] = {};
			} }, vnode->elm(), reinterpret_cast<std::uintptr_t>(vnode));

        for (const auto& it : callbacks) {
            if (!oldCallbacks.count(it.first) && it.first != "ref") {
                EM_ASM_({
					var key = Module['UTF8ToString']($1).replace(/^on/, "");
					var elm = Module['nodes'][$0];
					elm.addEventListener(
						key,
						Module['eventProxy'],
						false
					);
					elm['asmDomEvents'][key] = Module['eventProxy']; }, vnode->elm(), it.first.c_str());
            }
        }

        if (vnode->hash() & hasRef) {
            bool (*const* callback)(emscripten::val) = callbacks.at("ref").target<bool (*)(emscripten::val)>();
            bool (*const* oldCallback)(emscripten::val) = oldVnode->hash() & hasRef ? oldCallbacks.at("ref").target<bool (*)(emscripten::val)>() : nullptr;
            if (!callback || !oldCallback || *oldCallback != *callback) {
                if (oldVnode->hash() & hasRef) {
                    oldCallbacks.at("ref")(emscripten::val::null());
                }
                callbacks.at("ref")(emscripten::val::module_property("nodes")[vnode->elm()]);
            }
        } else if (oldVnode->hash() & hasRef) {
            oldCallbacks.at("ref")(emscripten::val::null());
        }
    }

}

void wasmdom::VNode::diff(const VNode* oldVnode) const
{
    const unsigned int vnodes = _hash | oldVnode->_hash;

    if (vnodes & hasAttrs)
        diffAttrs(oldVnode, this);
    if (vnodes & hasProps)
        diffProps(oldVnode, this);
    if (vnodes & hasCallbacks)
        diffCallbacks(oldVnode, this);
}

namespace wasmdom
{

    emscripten::val functionCallback(const std::uintptr_t& vnode, std::string callback, emscripten::val event)
    {
        const Callbacks& cbs = reinterpret_cast<VNode*>(vnode)->callbacks();
        if (!cbs.count(callback)) {
            callback = "on" + callback;
        }
        return emscripten::val(cbs.at(callback)(event));
    }

}

EMSCRIPTEN_BINDINGS(function_callback)
{
    emscripten::function("functionCallback", &wasmdom::functionCallback, emscripten::allow_raw_pointers());
}
