#include "wasm-dom/conf.h"

#include <algorithm>

WASMDOM_INLINE
wasmdom::VNode::VNode(std::nullptr_t) {}

WASMDOM_INLINE
wasmdom::VNode::VNode(const std::string& nodeSel)
    : _data(std::make_shared<SharedData>())
{
    _data->sel = nodeSel;
}

WASMDOM_INLINE
wasmdom::VNode::VNode(text_tag_t, const std::string& nodeText)
    : VNode(nodeText)
{
    normalize();
    // replace current type with text type
    _data->hash = (_data->hash & removeNodeType) | isText;
}

WASMDOM_INLINE
wasmdom::VNode::VNode(const std::string& nodeSel, const VNodeAttributes& nodeData)
    : VNode(nodeSel)
{
    _data->data = nodeData;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const std::string& nodeText)
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

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const VNode& child)
{
    _data->children.push_back(child);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const Children& nodeChildren)
{
    _data->children = nodeChildren;
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(std::initializer_list<VNode> nodeChildren)
{
    _data->children = nodeChildren;
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode::VNode(const VNode& other) = default;

WASMDOM_INLINE
wasmdom::VNode::VNode(VNode&& other) = default;

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator=(const VNode& other) = default;

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator=(VNode&& other) = default;

WASMDOM_INLINE
wasmdom::VNode::~VNode()
{
    if (_data.use_count() == 1) {
        // last vnode, update parent and children
        if (_data->parent && _data->parent->_data) {
            std::erase(_data->parent->_data->children, *this);
        }
        for (VNode& child : _data->children) {
            child._data->parent = nullptr;
        }
    }
}

WASMDOM_INLINE
const wasmdom::Attrs& wasmdom::VNode::attrs() const { return _data->data.attrs; }

#ifdef __EMSCRIPTEN__

WASMDOM_INLINE
const wasmdom::Props& wasmdom::VNode::props() const { return _data->data.props; }

WASMDOM_INLINE
const wasmdom::Callbacks& wasmdom::VNode::callbacks() const { return _data->data.callbacks; }

WASMDOM_INLINE
const wasmdom::EventCallbacks& wasmdom::VNode::eventCallbacks() const { return _data->data.eventCallbacks; }

#endif

WASMDOM_INLINE
const std::string& wasmdom::VNode::sel() const { return _data->sel; }

WASMDOM_INLINE
const std::string& wasmdom::VNode::key() const { return _data->key; }

WASMDOM_INLINE
const std::string& wasmdom::VNode::ns() const { return _data->ns; }

WASMDOM_INLINE
std::size_t wasmdom::VNode::hash() const { return _data->hash; }

#ifdef __EMSCRIPTEN__

WASMDOM_INLINE
void wasmdom::VNode::setNode(const emscripten::val& node) { _data->node = node; }

WASMDOM_INLINE
const emscripten::val& wasmdom::VNode::node() const { return _data->node; }

WASMDOM_INLINE
emscripten::val& wasmdom::VNode::node() { return _data->node; }

#endif

WASMDOM_INLINE
void wasmdom::VNode::updateParent(const VNode& oldVnode)
{
    if (_data->parent && _data->parent->_data) {
        std::erase(_data->parent->_data->children, *this);
    }

    const VNode& parent{ oldVnode.parent() };
    _data->parent = &parent;
    if (parent) {
        const Children::iterator vnodeIt{ std::ranges::find(parent._data->children, oldVnode) };
        if (vnodeIt != parent._data->children.end()) {
            *vnodeIt = *this;
        }
    }
}

WASMDOM_INLINE
const wasmdom::VNode& wasmdom::VNode::parent() const
{
    if (_data->parent && _data->parent->_data) {
        return *_data->parent;
    } else {
        static const VNode nullVnode{ nullptr };
        return nullVnode;
    }
}

WASMDOM_INLINE
void wasmdom::VNode::normalize() { normalize(false); }

WASMDOM_INLINE
wasmdom::VNode::operator bool() const { return _data != nullptr; }

WASMDOM_INLINE
bool wasmdom::VNode::operator!() const { return !static_cast<bool>(*this); }

WASMDOM_INLINE
bool wasmdom::VNode::operator==(const VNode& other) const { return _data == other._data; }

WASMDOM_INLINE
wasmdom::Children::iterator wasmdom::VNode::begin() { return _data->children.begin(); }

WASMDOM_INLINE
wasmdom::Children::iterator wasmdom::VNode::end() { return _data->children.end(); }

WASMDOM_INLINE
wasmdom::Children::const_iterator wasmdom::VNode::begin() const { return _data->children.begin(); }

WASMDOM_INLINE
wasmdom::Children::const_iterator wasmdom::VNode::end() const { return _data->children.end(); }
