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
        addChild(VNode(text_tag, nodeText));
        _data->hash |= hasText;
    }
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const VNode& child)
{
    addChild(child);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const Children& nodeChildren)
{
    _data->children.reserve(nodeChildren.size());
    for (const VNode& child : nodeChildren) {
        addChild(child);
    }
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(std::initializer_list<VNode> nodeChildren)
{
    _data->children.reserve(nodeChildren.size());
    for (const VNode& child : nodeChildren) {
        addChild(child);
    }
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode::VNode(const VNode& other)
    : _data(other._data)
{
}

WASMDOM_INLINE
wasmdom::VNode::VNode(VNode&& other)
    : _data(std::exchange(other._data, nullptr))
{
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator=(const VNode& other)
{
    std::shared_ptr tmp(other._data);
    std::swap(_data, tmp);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator=(VNode&& other)
{
    std::shared_ptr tmp(std::move(other._data));
    std::swap(_data, tmp);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode::~VNode()
{
    if (_data.use_count() == 1) {
        // last vnode, update parent and children
        if (_data->parent && _data->parent->_data) {
            _data->parent->removeChild(*this);
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
void wasmdom::VNode::removeChild(const VNode& child) { std::erase(_data->children, child); }

WASMDOM_INLINE
void wasmdom::VNode::addChild(const VNode& child)
{
    if (child) {
        _data->children.push_back(child);
        _data->children.back().setParent(*this);
    }
}

WASMDOM_INLINE
void wasmdom::VNode::addChild(VNode&& child)
{
    if (child) {
        child.setParent(*this);
        _data->children.push_back(std::move(child));
    }
}

WASMDOM_INLINE
void wasmdom::VNode::insertChild(const VNode& referenceChild, const VNode& child)
{
    if (!child)
        return;

    const Children::const_iterator childIt{ std::ranges::find(_data->children, child) };
    if (childIt != _data->children.end()) {
        _data->children.erase(childIt);
    }

    const Children::const_iterator referenceChildIt{ std::ranges::find(_data->children, referenceChild) };
    _data->children.insert(referenceChildIt, child)->setParent(*this);
}

WASMDOM_INLINE
void wasmdom::VNode::setParent(VNode& parent) { _data->parent = &parent; }

WASMDOM_INLINE
const wasmdom::VNode& wasmdom::VNode::parent() const
{
    if (_data->parent) {
        return *_data->parent;
    } else {
        static const VNode nullVnode{ nullptr };
        return nullVnode;
    }
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::parent()
{
    if (_data->parent) {
        return *_data->parent;
    } else {
        static VNode nullVnode{ nullptr };
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
