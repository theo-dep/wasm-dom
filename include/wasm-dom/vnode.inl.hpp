#include "wasm-dom/conf.h"

WASMDOM_INLINE
wasmdom::VNode::VNode(std::nullptr_t) {}

WASMDOM_INLINE
wasmdom::VNode::VNode(const std::string& nodeSel)
    : _data(std::make_shared<VNodeData>())
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
    _data->attrs = nodeData.attrs;
#ifdef __EMSCRIPTEN__
    _data->props = nodeData.props;
    _data->callbacks = nodeData.callbacks;
    _data->eventCallbacks = nodeData.eventCallbacks;
#endif
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const std::string& nodeText)
{
    normalize();
    if (_data->hash & isComment) {
        _data->sel = nodeText;
    } else {
        _children.emplace_back(text_tag, nodeText);
        _data->hash |= hasText;
    }
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const VNode& child)
{
    _children.push_back(child);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(const std::vector<VNode>& nodeChildren)
{
    insertChildren(nodeChildren.begin(), nodeChildren.end());
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator()(std::initializer_list<VNode> nodeChildren)
{
    insertChildren(nodeChildren.begin(), nodeChildren.end());
    return *this;
}

WASMDOM_INLINE
const wasmdom::Attrs& wasmdom::VNode::attrs() const { return _data->attrs; }

#ifdef __EMSCRIPTEN__

WASMDOM_INLINE
const wasmdom::Props& wasmdom::VNode::props() const { return _data->props; }

WASMDOM_INLINE
const wasmdom::Callbacks& wasmdom::VNode::callbacks() const { return _data->callbacks; }

WASMDOM_INLINE
const wasmdom::EventCallbacks& wasmdom::VNode::eventCallbacks() const { return _data->eventCallbacks; }

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
const emscripten::val& wasmdom::VNode::node() const { return _data->node; }

#endif

WASMDOM_INLINE
const std::vector<wasmdom::VNode>& wasmdom::VNode::children() const { return _children; }

WASMDOM_INLINE
void wasmdom::VNode::normalize() { normalize(false); }

WASMDOM_INLINE
bool wasmdom::VNode::valid() const { return _data != nullptr; }

template <typename Iterator>
WASMDOM_INLINE void wasmdom::VNode::insertChildren(Iterator begin, Iterator end)
{
    _children.reserve(_children.size() + std::distance(begin, end));
    for (; begin != end; ++begin) {
        if (begin->valid()) {
            _children.push_back(*begin);
        }
    }
}
