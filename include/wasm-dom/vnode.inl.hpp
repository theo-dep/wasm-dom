#include "wasm-dom/conf.h"

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
emscripten::val wasmdom::VNode::node() const { return wasmdom::internals::resolveNode(_data->node); }

WASMDOM_INLINE
emscripten::val wasmdom::VNode::parentNode() const { return wasmdom::internals::resolveNode(_data->parentNode); }

WASMDOM_INLINE
wasmdom::internals::NodeId wasmdom::VNode::nodeId() const { return _data->node; }

WASMDOM_INLINE
wasmdom::internals::NodeId wasmdom::VNode::parentNodeId() const { return _data->parentNode; }

WASMDOM_INLINE
void wasmdom::VNode::setNode(const emscripten::val& node)
{
    wasmdom::internals::dropNode(_data->node);
    _data->node = wasmdom::internals::allocNode(node);
}

WASMDOM_INLINE
void wasmdom::VNode::setParentNode(const emscripten::val& node)
{
    wasmdom::internals::dropNode(_data->parentNode);
    _data->parentNode = wasmdom::internals::allocNode(node);
}

WASMDOM_INLINE
void wasmdom::VNode::setNodeId(wasmdom::internals::NodeId id)
{
    if (id == _data->node)
        return;
    wasmdom::internals::retainNode(id);
    wasmdom::internals::dropNode(_data->node);
    _data->node = id;
}

WASMDOM_INLINE
void wasmdom::VNode::setParentNodeId(wasmdom::internals::NodeId id)
{
    if (id == _data->parentNode)
        return;
    wasmdom::internals::retainNode(id);
    wasmdom::internals::dropNode(_data->parentNode);
    _data->parentNode = id;
}

WASMDOM_INLINE
std::unordered_map<std::string, emscripten::val>& wasmdom::VNode::installedListeners() { return _data->installedListeners; }

WASMDOM_INLINE
const std::unordered_map<std::string, emscripten::val>& wasmdom::VNode::installedListeners() const { return _data->installedListeners; }

#endif

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
