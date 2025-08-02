// =============================================================================
// Single Header Library
// Auto-generated from multiple source files
// Project: wasm-dom
// =============================================================================
#pragma once

#include <concepts>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// -----------------------------------------------------------------------------
// src/wasm-dom/attribute.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

    template <typename T>
    concept Stringifiable = std::convertible_to<T, std::string>;

    template <typename T>
    concept StringAttribute = Stringifiable<T>;

    template <typename T>
    concept ValAttribute = std::convertible_to<T, emscripten::val>;

    template <typename T>
    concept CallbackAttribute = requires(T f) {
        { f(std::declval<emscripten::val>()) } -> std::convertible_to<bool>;
    };

    template <typename T>
    concept Attribute = StringAttribute<T> || ValAttribute<T> || CallbackAttribute<T>;

    using Callback = std::function<bool(emscripten::val)>;

    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;

    struct VNodeAttributes
    {
        Attrs attrs;
        Props props;
        Callbacks callbacks;
    };

    namespace detail
    {
        template <Stringifiable K, Attribute V>
        inline void attributeToVNode(VNodeAttributes& attributes, std::pair<K, V>&& attribute)
        {
            auto&& [key, value]{ attribute };
            if constexpr (StringAttribute<V>) {
                attributes.attrs.emplace(key, value);
            } else if constexpr (ValAttribute<V>) {
                attributes.props.emplace(key, value);
            } else if constexpr (CallbackAttribute<V>) {
                attributes.callbacks.emplace(key, value);
            } else {
                static_assert(false, "Type not supported");
            }
        }
    }

    template <Stringifiable... K, Attribute... V>
    inline VNodeAttributes attributesToVNode(std::pair<K, V>&&... attributes)
    {
        VNodeAttributes vnodeAttributes;
        (detail::attributeToVNode(vnodeAttributes, std::forward<std::pair<K, V>>(attributes)), ...);
        return vnodeAttributes;
    }

}

// -----------------------------------------------------------------------------
// src/wasm-dom/init.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

    void init();

}

// -----------------------------------------------------------------------------
// src/wasm-dom/vnode.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

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

    struct text_tag_t
    {
    };
    static inline constexpr text_tag_t text_tag{};

    class VNode
    {
        struct SharedData
        {
            std::string sel;
            std::string key;
            std::string ns;
            unsigned int hash = 0;
            VNodeAttributes data;
            int elm = 0;
            Children children;
        };

    public:
        VNode(std::nullptr_t);
        VNode(const std::string& nodeSel);
        VNode(text_tag_t, const std::string& nodeText);
        template <Stringifiable... K, Attribute... V>
        VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData);
        VNode(const std::string& nodeSel, const VNodeAttributes& nodeData);

        VNode& operator()(const std::string& nodeText);

        VNode& operator()(const Children::value_type& child);
        VNode& operator()(const Children& nodeChildren);
        VNode& operator()(std::initializer_list<Children::value_type> nodeChildren);

        const Attrs& attrs() const;
        const Props& props() const;
        const Callbacks& callbacks() const;

        const std::string& sel() const;
        const std::string& key() const;
        const std::string& ns() const;
        unsigned int hash() const;
        int elm() const;

        void setElm(int nodeElm);

        const Children& children() const;

        void normalize();

        operator bool() const;
        bool operator!() const;
        bool operator==(const VNode& other) const;

        std::string toHTML() const;

        void diff(const VNode& other) const;

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

template <wasmdom::Stringifiable... K, wasmdom::Attribute... V>
inline wasmdom::VNode::VNode(const std::string& nodeSel, std::pair<K, V>&&... nodeData)
    : VNode(nodeSel)
{
    _data->data = attributesToVNode(std::forward<std::pair<K, V>>(nodeData)...);
}

// -----------------------------------------------------------------------------
// src/wasm-dom/vdom.hpp
// -----------------------------------------------------------------------------
namespace emscripten
{
    class val;
}

namespace wasmdom
{

    class VNode;

    class VDom
    {
    public:
        VDom(const emscripten::val& element);

        const VNode& patch(const VNode& vnode);

    private:
        VNode _currentNode;
    };

}

// -----------------------------------------------------------------------------
// src/wasm-dom/init.cpp
// -----------------------------------------------------------------------------
void wasmdom::init()
{
    EM_ASM(
        Module['eventProxy'] = function(e) { return Module['functionCallback'](this['asmDomVNodeCallbacks'], e.type, e); };

        var recycler = Module['recycler'] = { 'nodes' : {} };
        recycler['create'] = function(name) {
                var list = recycler['nodes'][name.toUpperCase()];
                return list !== undefined && list.pop() || document.createElement(name); };
        recycler['createNS'] = function(name, ns) {
                var list = recycler['nodes'][name.toUpperCase() + ns];
                var node = list !== undefined && list.pop() || document.createElementNS(ns, name);
                node['asmDomNS'] = ns;
                return node; };
        recycler['createText'] = function(text) {
                var list = recycler['nodes']['#TEXT'];
                if (list !== undefined) {
                    var node = list.pop();
                    if (node !== undefined) {
                        node.nodeValue = text;
                        return node;
                    }
                }
                return document.createTextNode(text); };
        recycler['createComment'] = function(comment) {
                var list = recycler['nodes']['#COMMENT'];
                if (list !== undefined) {
                    var node = list.pop();
                    if (node !== undefined) {
                        node.nodeValue = comment;
                        return node;
                    }
                }
                return document.createComment(comment); };
        recycler['collect'] = function(node) {
                // clean
                var i;

                while (i = node.lastChild) {
                    node.removeChild(i);
                    recycler['collect'](i);
                }
                i = node.attributes !== undefined ? node.attributes.length : 0;
                while (i--) node.removeAttribute(node.attributes[i].name);
                node['asmDomVNodeCallbacks'] = undefined;
                if (node['asmDomRaws'] !== undefined) {
                    node['asmDomRaws'].forEach(function(raw) {
                        node[raw] = undefined;
                    });
                    node['asmDomRaws'] = undefined;
                }
                if (node['asmDomEvents'] !== undefined) {
                    Object.keys(node['asmDomEvents']).forEach(function(event) {
                        node.removeEventListener(event, node['asmDomEvents'][event], false);
                    });
                    node['asmDomEvents'] = undefined;
                }
                if (node.nodeValue !== null && node.nodeValue !== "") {
                    node.nodeValue = "";
                }
                Object.keys(node).forEach(function(key) {
                    if (
                        key[0] !== 'a' || key[1] !== 's' || key[2] !== 'm' ||
                        key[3] !== 'D' || key[4] !== 'o' || key[5] !== 'm'
                    ) {
                        node[key] = undefined;
                    }
                });

                // collect
                var name = node.nodeName.toUpperCase();
                if (node['asmDomNS'] !== undefined) name += node.namespaceURI;
                var list = recycler['nodes'][name];
                if (list !== undefined) list.push(node);
                else recycler['nodes'][name] = [node]; };

        var nodes = Module['nodes'] = { 0 : null };
        var lastPtr = 0;

        function addPtr(node) {
            // clang-format off
            if (node === null)
                return 0;
            if (node['asmDomPtr'] !== undefined)
                return node['asmDomPtr'];
            // clang-format on
            nodes[++lastPtr] = node;
            return node['asmDomPtr'] = lastPtr;
        };

        Module['addNode'] = function(node) {
                addPtr(node.parentNode);
                addPtr(node.nextSibling);
                return addPtr(node); };
        Module.createElement = function(tagName) { return addPtr(recycler['create'](tagName)); };
        Module.createElementNS = function(namespaceURI, qualifiedName) { return addPtr(recycler['createNS'](qualifiedName, namespaceURI)); };
        Module.createTextNode = function(text) { return addPtr(recycler['createText'](text)); };
        Module.createComment = function(text) { return addPtr(recycler['createComment'](text)); };
        Module.createDocumentFragment = function() { return addPtr(document.createDocumentFragment()); };
        Module.insertBefore = function(parentNodePtr, newNodePtr, referenceNodePtr) { nodes[parentNodePtr].insertBefore(
                                                                                          nodes[newNodePtr],
                                                                                          nodes[referenceNodePtr]
                                                                                      ); };
        Module.removeChild = function(childPtr) {
                var node = nodes[childPtr];
                if (node === null || node === undefined) return;
                var parent = node.parentNode;
                if (parent !== null) parent.removeChild(node);
                recycler['collect'](node); };
        Module.appendChild = function(parentPtr, childPtr) { nodes[parentPtr].appendChild(nodes[childPtr]); };
        Module.removeAttribute = function(nodePtr, attr) { nodes[nodePtr].removeAttribute(attr); };
        Module.setAttribute = function(nodePtr, attr, value) {
                // xChar = 120
                // colonChar = 58
                if (attr.charCodeAt(0) !== 120) {
                    nodes[nodePtr].setAttribute(attr, value);
                } else if (attr.charCodeAt(3) === 58) {
                    // Assume xml namespace
                    nodes[nodePtr].setAttributeNS('http://www.w3.org/XML/1998/namespace', attr, value);
                } else if (attr.charCodeAt(5) === 58) {
                    // Assume xlink namespace
                    nodes[nodePtr].setAttributeNS('http://www.w3.org/1999/xlink', attr, value);
                } else {
                    nodes[nodePtr].setAttribute(attr, value);
                } };
        Module.parentNode = function(nodePtr) {
                var node = nodes[nodePtr];
                return (
                    node !== null && node !== undefined &&
                    node.parentNode !== null
                ) ? node.parentNode['asmDomPtr'] : 0; };
        Module.nextSibling = function(nodePtr) {
                var node = nodes[nodePtr];
                return (
                    node !== null && node !== undefined &&
                    node.nextSibling !== null
                ) ? node.nextSibling['asmDomPtr'] : 0; };
        Module.setNodeValue = function(nodePtr, text) { nodes[nodePtr].nodeValue = text; };
    );
}

// -----------------------------------------------------------------------------
// src/wasm-dom/vdom.cpp
// -----------------------------------------------------------------------------
wasmdom::VDom::VDom(const emscripten::val& element)
    : _currentNode(VNode::toVNode(element))
{
    _currentNode.normalize();
}

namespace wasmdom
{

    void patchVNode(VNode& oldVnode, VNode& vnode, int parentElm);

    bool sameVNode(const VNode& vnode1, const VNode& vnode2)
    {
        return
            // compare selector, nodeType and key existance
            ((vnode1.hash() & id) == (vnode2.hash() & id)) &&
            // compare keys
            (!(vnode1.hash() & hasKey) || (vnode1.key() == vnode2.key()));
    }

    int createElm(VNode& vnode)
    {
        if (vnode.hash() & isElement) {
            vnode.setElm(EM_ASM_INT(
                { return
                      // clang-format off
                      $1 === 0
                               // clang-format on
                               ? Module.createElement(
                                     Module['UTF8ToString']($0)
                                 )
                               : Module.createElementNS(
                                     Module['UTF8ToString']($1),
                                     Module['UTF8ToString']($0)
                                 ); }, vnode.sel().c_str(), vnode.hash() & hasNS ? vnode.ns().c_str() : 0
            ));
        } else if (vnode.hash() & isText) {
            vnode.setElm(EM_ASM_INT({ return Module.createTextNode(
                                          Module['UTF8ToString']($0)
                                      ); }, vnode.sel().c_str()));
            return vnode.elm();
        } else if (vnode.hash() & isFragment) {
            vnode.setElm(EM_ASM_INT({
                return Module.createDocumentFragment();
            }));
        } else if (vnode.hash() & isComment) {
            vnode.setElm(EM_ASM_INT({ return Module.createComment(
                                          Module['UTF8ToString']($0)
                                      ); }, vnode.sel().c_str()));
            return vnode.elm();
        }

        for (VNode& child : vnode._data->children) {
            int elm = createElm(child);
            EM_ASM({ Module.appendChild($0, $1); }, vnode.elm(), elm);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);

        return vnode.elm();
    }

    void addVNodes(int parentElm, int before, Children& vnodes, Children::size_type startIdx, Children::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            int elm = createElm(vnodes[startIdx++]);
            EM_ASM({ Module.insertBefore($0, $1, $2) }, parentElm, elm, before);
        }
    }

    void removeVNodes(const Children& vnodes, Children::size_type startIdx, Children::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            const VNode& vnode = vnodes[startIdx++];

            if (vnode) {
                EM_ASM({ Module.removeChild($0); }, vnode.elm());

                if (vnode.hash() & hasRef) {
                    vnode.callbacks().at("ref")(emscripten::val::null());
                }
            }
        }
    }

    VNode boundCheckVNode(Children& ch, int idx, int endIdx)
    {
        return (idx >= 0 && idx <= endIdx ? ch[idx] : nullptr);
    }

    void updateChildren(int parentElm, Children& oldCh, Children& newCh)
    {
        int oldStartIdx = 0;
        int newStartIdx = 0;
        int oldEndIdx = oldCh.size() - 1;
        int newEndIdx = newCh.size() - 1;
        const int& oldMaxIdx = oldEndIdx; // to avoid unsequenced modification and access
        const int& newMaxIdx = newEndIdx; // |
        VNode oldStartVnode = oldCh[0];
        VNode oldEndVnode = oldCh[oldEndIdx];
        VNode newStartVnode = newCh[0];
        VNode newEndVnode = newCh[newEndIdx];
        bool oldKeys = false;
        std::unordered_map<std::string, int> oldKeyToIdx;

        while (oldStartIdx <= oldEndIdx & newStartIdx <= newEndIdx) {
            if (!oldStartVnode) {
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
            } else if (!oldEndVnode) {
                oldEndVnode = boundCheckVNode(oldCh, --oldEndIdx, oldMaxIdx);
            } else if (sameVNode(oldStartVnode, newStartVnode)) {
                if (oldStartVnode != newStartVnode)
                    patchVNode(oldStartVnode, newStartVnode, parentElm);
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
                newStartVnode = boundCheckVNode(newCh, ++newStartIdx, newMaxIdx);
            } else if (sameVNode(oldEndVnode, newEndVnode)) {
                if (oldEndVnode != newEndVnode)
                    patchVNode(oldEndVnode, newEndVnode, parentElm);
                oldEndVnode = boundCheckVNode(oldCh, --oldEndIdx, oldMaxIdx);
                newEndVnode = boundCheckVNode(newCh, --newEndIdx, newMaxIdx);
            } else if (sameVNode(oldStartVnode, newEndVnode)) {
                if (oldStartVnode != newEndVnode)
                    patchVNode(oldStartVnode, newEndVnode, parentElm);

                EM_ASM({ Module.insertBefore(
                             $0,
                             $1,
                             Module.nextSibling($2)
                         ); }, parentElm, oldStartVnode.elm(), oldEndVnode.elm());
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
                newEndVnode = boundCheckVNode(newCh, --newEndIdx, newMaxIdx);
            } else if (sameVNode(oldEndVnode, newStartVnode)) {
                if (oldEndVnode != newStartVnode)
                    patchVNode(oldEndVnode, newStartVnode, parentElm);

                EM_ASM({ Module.insertBefore($0, $1, $2); }, parentElm, oldEndVnode.elm(), oldStartVnode.elm());
                oldEndVnode = boundCheckVNode(oldCh, --oldEndIdx, oldMaxIdx);
                newStartVnode = boundCheckVNode(newCh, ++newStartIdx, newMaxIdx);
            } else {
                if (!oldKeys) {
                    oldKeys = true;
                    int beginIdx = oldStartIdx;
                    while (beginIdx <= oldEndIdx) {
                        if (oldCh[beginIdx].hash() & hasKey) {
                            oldKeyToIdx.emplace(oldCh[beginIdx].key(), beginIdx);
                        }
                        ++beginIdx;
                    }
                }
                if (!oldKeyToIdx.contains(newStartVnode.key())) {
                    int elm = createElm(newStartVnode);
                    EM_ASM({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode.elm());
                } else {
                    VNode elmToMove = oldCh[oldKeyToIdx[newStartVnode.key()]];
                    if ((elmToMove.hash() & extractSel) != (newStartVnode.hash() & extractSel)) {
                        int elm = createElm(newStartVnode);
                        EM_ASM({ Module.insertBefore($0, $1, $2); }, parentElm, elm, oldStartVnode.elm());
                    } else {
                        if (elmToMove != newStartVnode)
                            patchVNode(elmToMove, newStartVnode, parentElm);
                        oldCh[oldKeyToIdx[newStartVnode.key()]] = nullptr;
                        EM_ASM({ Module.insertBefore($0, $1, $2); }, parentElm, elmToMove.elm(), oldStartVnode.elm());
                    }
                }
                newStartVnode = boundCheckVNode(newCh, ++newStartIdx, newMaxIdx);
            }
        }
        if (oldStartIdx <= oldEndIdx || newStartIdx <= newEndIdx) {
            if (oldStartIdx > oldEndIdx) {
                addVNodes(parentElm, newEndIdx + 1 <= static_cast<int>(newCh.size()) - 1 ? newCh[newEndIdx + 1].elm() : 0, newCh, newStartIdx, newEndIdx);
            } else {
                removeVNodes(oldCh, oldStartIdx, oldEndIdx);
            }
        }
    }

}

void wasmdom::patchVNode(VNode& oldVnode, VNode& vnode, int parentElm)
{
    vnode.setElm(oldVnode.elm());
    if (vnode.hash() & isElementOrFragment) {
        const unsigned int childrenNotEmpty = vnode.hash() & hasChildren;
        const unsigned int oldChildrenNotEmpty = oldVnode.hash() & hasChildren;
        if (childrenNotEmpty && oldChildrenNotEmpty) {
            updateChildren(vnode.hash() & isFragment ? parentElm : vnode.elm(), oldVnode._data->children, vnode._data->children);
        } else if (childrenNotEmpty) {
            addVNodes(vnode.hash() & isFragment ? parentElm : vnode.elm(), 0, vnode._data->children, 0, vnode.children().size() - 1);
        } else if (oldChildrenNotEmpty) {
            removeVNodes(oldVnode._data->children, 0, oldVnode.children().size() - 1);
        }
        vnode.diff(oldVnode);
    } else if (vnode.sel() != oldVnode.sel()) {
        EM_ASM({ Module.setNodeValue(
                     $0,
                     Module['UTF8ToString']($1)
                 ); }, vnode.elm(), vnode.sel().c_str());
    }
}

const wasmdom::VNode& wasmdom::VDom::patch(const VNode& vnode)
{
    if (!vnode || _currentNode == vnode)
        return _currentNode;

    VNode newNode = vnode;
    newNode.normalize();

    if (sameVNode(_currentNode, newNode)) {
        patchVNode(_currentNode, newNode, _currentNode.elm());
    } else {
        int elm = createElm(newNode);
        EM_ASM({
                var parent = Module.parentNode($1);
                if (parent !== 0) {
                    Module.insertBefore(
                        parent,
                        $0,
                        Module.nextSibling($1)
                    );
                    Module.removeChild($1);
                } }, elm, _currentNode.elm());
    }

    _currentNode = newNode;

    return _currentNode;
}

// -----------------------------------------------------------------------------
// src/wasm-dom/vnode.cpp
// -----------------------------------------------------------------------------
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
                EM_ASM({ Module.removeAttribute(
                             $0,
                             Module['UTF8ToString']($1)
                         ); }, vnode.elm(), key.c_str());
            }
        }

        for (const auto& [key, val] : attrs) {
            if (!oldAttrs.contains(key) || oldAttrs.at(key) != val) {
                EM_ASM({ Module.setAttribute(
                             $0,
                             Module['UTF8ToString']($1),
                             Module['UTF8ToString']($2)
                         ); }, vnode.elm(), key.c_str(), val.c_str());
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

// -----------------------------------------------------------------------------
// src/wasm-dom/vnode.inl.cpp
// -----------------------------------------------------------------------------
inline wasmdom::VNode::VNode(std::nullptr_t) {}

inline wasmdom::VNode::VNode(const std::string& nodeSel)
    : _data(std::make_shared<SharedData>())
{
    _data->sel = nodeSel;
}

inline wasmdom::VNode::VNode(text_tag_t, const std::string& nodeText)
    : _data(std::make_shared<SharedData>())
{
    normalize();
    _data->sel = nodeText;
    // replace current type with text type
    _data->hash = (_data->hash & removeNodeType) | isText;
}

inline wasmdom::VNode::VNode(const std::string& nodeSel, const VNodeAttributes& nodeData)
    : VNode(nodeSel)
{
    _data->data = nodeData;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(const std::string& nodeText)
{
    normalize();
    if (_data->hash & isComment) {
        _data->sel = nodeText;
    } else {
        _data->children.emplace_back(text_tag, nodeText);
        _data->hash |= hasText;
    }
    return *this;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(const Children::value_type& child)
{
    _data->children.push_back(child);
    return *this;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(const Children& nodeChildren)
{
    _data->children = nodeChildren;
    return *this;
}

inline wasmdom::VNode& wasmdom::VNode::operator()(std::initializer_list<Children::value_type> nodeChildren)
{
    _data->children = nodeChildren;
    return *this;
}

inline const wasmdom::Attrs& wasmdom::VNode::attrs() const { return _data->data.attrs; }

inline const wasmdom::Props& wasmdom::VNode::props() const { return _data->data.props; }

inline const wasmdom::Callbacks& wasmdom::VNode::callbacks() const { return _data->data.callbacks; }

inline const std::string& wasmdom::VNode::sel() const { return _data->sel; }

inline const std::string& wasmdom::VNode::key() const { return _data->key; }

inline const std::string& wasmdom::VNode::ns() const { return _data->ns; }

inline unsigned int wasmdom::VNode::hash() const { return _data->hash; }

inline int wasmdom::VNode::elm() const { return _data->elm; }

inline void wasmdom::VNode::setElm(int nodeElm) { _data->elm = nodeElm; }

inline const wasmdom::Children& wasmdom::VNode::children() const { return _data->children; }

inline void wasmdom::VNode::normalize() { normalize(false); }

inline wasmdom::VNode::operator bool() const { return _data != nullptr; }

inline bool wasmdom::VNode::operator!() const { return _data == nullptr; }

inline bool wasmdom::VNode::operator==(const VNode& other) const { return _data == other._data; }

// End of single header library
