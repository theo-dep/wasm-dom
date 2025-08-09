// =============================================================================
// Single Header Library
// Auto-generated from multiple source files
// Project: wasm-dom
// =============================================================================
#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <functional>
#include <memory>
#include <ranges>
#include <regex>
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
// src/wasm-dom/domapi.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

    class VNode;

    namespace domapi
    {
        emscripten::val node(int nodePtr);

        int addNode(const emscripten::val& node);

        int createElement(const std::string& tag);
        int createElementNS(const std::string& namespaceURI, const std::string& qualifiedName);
        int createTextNode(const std::string& text);
        int createComment(const std::string& comment);
        int createDocumentFragment();

        void insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr);
        void removeChild(int childPtr);
        void appendChild(int parentPtr, int childPtr);
        void removeAttribute(int nodePtr, const std::string& attribute);
        void setAttribute(int nodePtr, const std::string& attribute, const std::string& value);
        void setNodeValue(int nodePtr, const std::string& text);

        int parentNode(int nodePtr);
        int nextSibling(int nodePtr);
    }
}

// -----------------------------------------------------------------------------
// src/wasm-dom/domrecycler.hpp
// -----------------------------------------------------------------------------
namespace wasmdom
{

    class DomRecycler
    {
    public:
        DomRecycler(bool useWasmGC);
        ~DomRecycler();

        emscripten::val create(const std::string& name);
        emscripten::val createNS(const std::string& name, const std::string& ns);
        emscripten::val createText(const std::string& text);
        emscripten::val createComment(const std::string& comment);

        void collect(emscripten::val node);

        // valid if no garbage collector
        std::vector<emscripten::val> nodes(const std::string& name) const;

    private:
        struct DomFactory;
        struct DomRecyclerFactory;
        DomFactory* _d_ptr;
    };

    DomRecycler& recycler();

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
// src/wasm-dom/domapi.cpp
// -----------------------------------------------------------------------------
namespace wasmdom::domapi
{
    std::unordered_map<int, emscripten::val>& nodes()
    {
        static std::unordered_map<int, emscripten::val> nodes{ { 0, emscripten::val::null() } };
        return nodes;
    }

    int addPtr(const emscripten::val& node)
    {
        static int lastPtr = 0;

        if (node.isNull() || node.isUndefined())
            return 0;
        if (!node["asmDomPtr"].isUndefined())
            return node["asmDomPtr"].as<int>();

        emscripten::val newNode = node;

        ++lastPtr;
        newNode.set("asmDomPtr", lastPtr);
        nodes().emplace(lastPtr, newNode);
        return lastPtr;
    }
}

emscripten::val wasmdom::domapi::node(int nodePtr)
{
    if (nodes().contains(nodePtr))
        return nodes()[nodePtr];
    return emscripten::val::null();
}

int wasmdom::domapi::addNode(const emscripten::val& node)
{
    addPtr(node["parentNode"]);
    addPtr(node["nextSibling"]);
    return addPtr(node);
}

int wasmdom::domapi::createElement(const std::string& tag)
{
    return addPtr(recycler().create(tag));
}

int wasmdom::domapi::createElementNS(const std::string& namespaceURI, const std::string& qualifiedName)
{
    return addPtr(recycler().createNS(qualifiedName, namespaceURI));
}

int wasmdom::domapi::createTextNode(const std::string& text)
{
    return addPtr(recycler().createText(text));
}

int wasmdom::domapi::createComment(const std::string& comment)
{
    return addPtr(recycler().createComment(comment));
}

int wasmdom::domapi::createDocumentFragment()
{
    return addPtr(emscripten::val::global("document").call<emscripten::val>("createDocumentFragment"));
}

void wasmdom::domapi::insertBefore(int parentNodePtr, int newNodePtr, int referenceNodePtr)
{
    if (parentNodePtr == 0 /*|| newNodePtr == 0 || referenceNodePtr == 0*/)
        return;

    node(parentNodePtr).call<void>("insertBefore", node(newNodePtr), node(referenceNodePtr));
}

void wasmdom::domapi::removeChild(int childPtr)
{
    emscripten::val node = domapi::node(childPtr);
    if (node.isNull() || node.isUndefined())
        return;

    emscripten::val parentNode = node["parentNode"];
    if (!parentNode.isNull())
        parentNode.call<void>("removeChild", node);

    recycler().collect(node);
}

void wasmdom::domapi::appendChild(int parentPtr, int childPtr)
{
    node(parentPtr).call<void>("appendChild", node(childPtr));
}

void wasmdom::domapi::removeAttribute(int nodePtr, const std::string& attribute)
{
    node(nodePtr).call<void>("removeAttribute", attribute);
}

void wasmdom::domapi::setAttribute(int nodePtr, const std::string& attribute, const std::string& value)
{
    emscripten::val node = domapi::node(nodePtr);
    if (attribute.starts_with("xml:")) {
        node.call<void>("setAttributeNS", std::string("http://www.w3.org/XML/1998/namespace"), attribute, value);
    } else if (attribute.starts_with("xlink:")) {
        node.call<void>("setAttributeNS", std::string("http://www.w3.org/1999/xlink"), attribute, value);
    } else {
        node.call<void>("setAttribute", attribute, value);
    }
}

void wasmdom::domapi::setNodeValue(int nodePtr, const std::string& text)
{
    node(nodePtr).set("nodeValue", text);
}

int wasmdom::domapi::parentNode(int nodePtr)
{
    emscripten::val node = domapi::node(nodePtr);
    if (!node.isNull() && !node.isUndefined() && !node["parentNode"].isNull())
        return node["parentNode"]["asmDomPtr"].as<int>();
    return 0;
}

int wasmdom::domapi::nextSibling(int nodePtr)
{
    emscripten::val node = domapi::node(nodePtr);
    if (!node.isNull() && !node.isUndefined() && !node["nextSibling"].isNull())
        return node["nextSibling"]["asmDomPtr"].as<int>();
    return 0;
}

// -----------------------------------------------------------------------------
// src/wasm-dom/domrecycler.cpp
// -----------------------------------------------------------------------------
wasmdom::DomRecycler& wasmdom::recycler()
{
    static DomRecycler recycler(true);
    return recycler;
}

namespace wasmdom
{
    EM_JS(bool, testGC, (), {
        // https://github.com/GoogleChromeLabs/wasm-feature-detect/blob/main/src/detectors/gc/index.js
        return WebAssembly.validate(new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0, 1, 5, 1, 95, 1, 120, 0]));
    })

    std::string upper(const std::string& str)
    {
        static const auto toupper{
            [](const std::string::value_type& c) -> std::string::value_type {
                return static_cast<std::string::value_type>(std::toupper(c));
            }
        };
        std::string upperStr = str;
        std::ranges::copy(std::views::transform(str, toupper), upperStr.begin());
        return upperStr;
    }

    struct DomRecycler::DomFactory
    {
        virtual ~DomFactory() = default;

        virtual emscripten::val create(const std::string& name)
        {
            return emscripten::val::global("document").call<emscripten::val>("createElement", name);
        }
        virtual emscripten::val createNS(const std::string& name, const std::string& ns)
        {
            emscripten::val node = emscripten::val::global("document").call<emscripten::val>("createElementNS", ns, name);
            node.set("asmDomNS", ns);
            return node;
        }
        virtual emscripten::val createText(const std::string& text)
        {
            return emscripten::val::global("document").call<emscripten::val>("createTextNode", text);
        }
        virtual emscripten::val createComment(const std::string& comment)
        {
            return emscripten::val::global("document").call<emscripten::val>("createComment", comment);
        }

        virtual void collect(emscripten::val /*node*/) {}
    };

    struct DomRecycler::DomRecyclerFactory : DomRecycler::DomFactory
    {
        std::unordered_map<std::string, std::vector<emscripten::val>> _nodes;

        emscripten::val create(const std::string& name) override;
        emscripten::val createNS(const std::string& name, const std::string& ns) override;
        emscripten::val createText(const std::string& text) override;
        emscripten::val createComment(const std::string& comment) override;

        void collect(emscripten::val node) override;
    };
}

wasmdom::DomRecycler::DomRecycler(bool useWasmGC)
    : _d_ptr{ testGC() && useWasmGC ? new DomFactory : new DomRecyclerFactory }
{
}

wasmdom::DomRecycler::~DomRecycler()
{
    delete _d_ptr;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::create(const std::string& name)
{
    std::vector<emscripten::val>& list = _nodes[upper(name)];

    if (list.empty())
        return DomFactory::create(name);

    emscripten::val node = list.back();
    list.pop_back();
    return node;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::createNS(const std::string& name, const std::string& ns)
{
    std::vector<emscripten::val>& list = _nodes[upper(name) + ns];

    emscripten::val node;
    if (list.empty()) {
        node = DomFactory::createNS(name, ns);
    } else {
        node = list.back();
        list.pop_back();
    }

    return node;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::createText(const std::string& text)
{
    constexpr const char* textKey = "#TEXT";
    std::vector<emscripten::val>& list = _nodes[textKey];

    if (list.empty())
        return DomFactory::createText(text);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", text);
    return node;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::createComment(const std::string& comment)
{
    constexpr const char* commentKey = "#COMMENT";
    std::vector<emscripten::val>& list = _nodes[commentKey];

    if (list.empty())
        return DomFactory::createComment(comment);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", comment);
    return node;
}

void wasmdom::DomRecycler::DomRecyclerFactory::collect(emscripten::val node)
{
    // clean
    for (emscripten::val child = node["lastChild"]; !child.isNull(); child = node["lastChild"]) {
        node.call<void>("removeChild", child);
        collect(child);
    }

    if (!node["attributes"].isUndefined()) {
        for (int i = node["attributes"]["length"].as<int>() - 1; i >= 0; --i) {
            node.call<void>("removeAttribute", node["attributes"][i]["name"]);
        }
    }

    node.set("asmDomVNodeCallbacks", emscripten::val::undefined());

    if (!node["asmDomRaws"].isUndefined()) {
        for (int i = 0; i < node["asmDomRaws"]["length"].as<int>(); ++i) {
            node.set(node["asmDomRaws"][i], emscripten::val::undefined());
        }
        node.set("asmDomRaws", emscripten::val::undefined());
    }

    if (!node["asmDomEvents"].isUndefined()) {
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", node["asmDomEvents"]);
        for (int i = 0; i < keys["length"].as<int>(); ++i) {
            emscripten::val event = keys[i];
            node.call<void>("removeEventListener", event, node["asmDomEvents"][event], false);
        }
        node.set("asmDomEvents", emscripten::val::undefined());
    }

    if (!node["nodeValue"].isNull() && !node["nodeValue"].as<std::string>().empty()) {
        node.set("nodeValue", std::string{});
    }

    emscripten::val nodeKeys = emscripten::val::global("Object").call<emscripten::val>("keys", node);
    for (int i = 0; i < nodeKeys["length"].as<int>(); ++i) {
        std::string key = nodeKeys[i].as<std::string>();
        if (!key.starts_with("asmDom")) {
            node.set(key, emscripten::val::undefined());
        }
    }

    // collect
    std::string nodeName = upper(node["nodeName"].as<std::string>());
    if (!node["asmDomNS"].isUndefined()) {
        nodeName += node["namespaceURI"].as<std::string>();
    }

    std::vector<emscripten::val>& list = _nodes[nodeName];
    list.push_back(node);
}

emscripten::val wasmdom::DomRecycler::create(const std::string& name)
{
    return _d_ptr->create(name);
}

emscripten::val wasmdom::DomRecycler::createNS(const std::string& name, const std::string& ns)
{
    return _d_ptr->createNS(name, ns);
}

emscripten::val wasmdom::DomRecycler::createText(const std::string& text)
{
    return _d_ptr->createText(text);
}

emscripten::val wasmdom::DomRecycler::createComment(const std::string& comment)
{
    return _d_ptr->createComment(comment);
}

void wasmdom::DomRecycler::collect(emscripten::val node)
{
    _d_ptr->collect(node);
}

std::vector<emscripten::val> wasmdom::DomRecycler::nodes(const std::string& name) const
{
    DomRecyclerFactory* ptr{ dynamic_cast<DomRecyclerFactory*>(_d_ptr) };
    if (ptr && ptr->_nodes.contains(name))
        return ptr->_nodes.at(name);
    return {};
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
            if (vnode.hash() & hasNS) {
                vnode.setElm(domapi::createElementNS(vnode.ns(), vnode.sel()));
            } else {
                vnode.setElm(domapi::createElement(vnode.sel()));
            }
        } else if (vnode.hash() & isText) {
            vnode.setElm(domapi::createTextNode(vnode.sel()));
            return vnode.elm();
        } else if (vnode.hash() & isFragment) {
            vnode.setElm(domapi::createDocumentFragment());
        } else if (vnode.hash() & isComment) {
            vnode.setElm(domapi::createComment(vnode.sel()));
            return vnode.elm();
        }

        for (VNode& child : vnode._data->children) {
            int elm = createElm(child);
            domapi::appendChild(vnode.elm(), elm);
        }

        static const VNode emptyNode("");
        vnode.diff(emptyNode);

        return vnode.elm();
    }

    void addVNodes(int parentElm, int before, Children& vnodes, Children::size_type startIdx, Children::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            int elm = createElm(vnodes[startIdx++]);
            domapi::insertBefore(parentElm, elm, before);
        }
    }

    void removeVNodes(const Children& vnodes, Children::size_type startIdx, Children::size_type endIdx)
    {
        while (startIdx <= endIdx) {
            const VNode& vnode = vnodes[startIdx++];

            if (vnode) {
                domapi::removeChild(vnode.elm());

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
                int nextSiblingOldPtr = domapi::nextSibling(oldEndVnode.elm());
                domapi::insertBefore(parentElm, oldStartVnode.elm(), nextSiblingOldPtr);
                oldStartVnode = boundCheckVNode(oldCh, ++oldStartIdx, oldMaxIdx);
                newEndVnode = boundCheckVNode(newCh, --newEndIdx, newMaxIdx);
            } else if (sameVNode(oldEndVnode, newStartVnode)) {
                if (oldEndVnode != newStartVnode)
                    patchVNode(oldEndVnode, newStartVnode, parentElm);

                domapi::insertBefore(parentElm, oldEndVnode.elm(), oldStartVnode.elm());
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
                    domapi::insertBefore(parentElm, elm, oldStartVnode.elm());
                } else {
                    VNode elmToMove = oldCh[oldKeyToIdx[newStartVnode.key()]];
                    if ((elmToMove.hash() & extractSel) != (newStartVnode.hash() & extractSel)) {
                        int elm = createElm(newStartVnode);
                        domapi::insertBefore(parentElm, elm, oldStartVnode.elm());
                    } else {
                        if (elmToMove != newStartVnode)
                            patchVNode(elmToMove, newStartVnode, parentElm);
                        oldCh[oldKeyToIdx[newStartVnode.key()]] = nullptr;
                        domapi::insertBefore(parentElm, elmToMove.elm(), oldStartVnode.elm());
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
        domapi::setNodeValue(vnode.elm(), vnode.sel());
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
        int parentPtr = domapi::parentNode(_currentNode.elm());
        int nextSiblingElmPtr = domapi::nextSibling(_currentNode.elm());
        domapi::insertBefore(parentPtr, elm, nextSiblingElmPtr);
        domapi::removeChild(_currentNode.elm());
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

namespace wasmdom
{
    void lower(std::string& str)
    {
        static const auto tolower{
            [](const std::string::value_type& c) -> std::string::value_type {
                return static_cast<std::string::value_type>(std::tolower(c));
            }
        };
        std::ranges::copy(std::views::transform(str, tolower), str.begin());
    }
}

wasmdom::VNode wasmdom::VNode::toVNode(const emscripten::val& node)
{
    VNode vnode = nullptr;
    int nodeType = node["nodeType"].as<int>();
    // isElement
    if (nodeType == 1) {
        std::string sel = node["tagName"].as<std::string>();
        lower(sel);

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
            if (std::ranges::find(omitProps, key) == omitProps.cend()) {
                std::string lowerKey(key);
                lower(lowerKey);
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
            bool isSvgContainerElement = isSvg && std::ranges::find(containerElements, vnode.sel()) != containerElements.cend();

            html.append("<" + vnode.sel());
            appendAttributes(vnode, html);
            if (isSvg && !isSvgContainerElement) {
                html.append(" /");
            }
            html.append(">");

            if (isSvgContainerElement ||
                (!isSvg && std::ranges::find(voidElements, vnode.sel()) == voidElements.cend())) {

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

        node.set("asmDomVNodeCallbacksKey", storeCallbacks(oldVnode, vnode));
        if (node["asmDomEvents"].isUndefined()) {
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

    bool eventProxy(emscripten::val event)
    {
        int nodePtr = event["currentTarget"]["asmDomVNodeCallbacksKey"].as<int>();
        std::string eventType = event["type"].as<std::string>();

        const Callbacks& callbacks = vnodeCallbacks()[nodePtr];
        if (!callbacks.contains(eventType)) {
            eventType = "on" + eventType;
        }
        return callbacks.at(eventType)(event);
    }

}

EMSCRIPTEN_BINDINGS(wasmdomEventModule)
{
    emscripten::function("eventProxy", wasmdom::eventProxy);
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
