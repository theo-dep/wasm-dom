#ifdef WASMDOM_COVERAGE
#define WASMDOM_INLINE
#else
#define WASMDOM_INLINE inline
#endif

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
    : _data(std::make_shared<SharedData>())
{
    normalize();
    _data->sel = nodeText;
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
wasmdom::VNode& wasmdom::VNode::operator()(const Children::value_type& child)
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
wasmdom::VNode& wasmdom::VNode::operator()(std::initializer_list<Children::value_type> nodeChildren)
{
    _data->children = nodeChildren;
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
    std::exchange(_data, other._data);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode& wasmdom::VNode::operator=(VNode&& other)
{
    std::swap(_data, other._data);
    return *this;
}

WASMDOM_INLINE
wasmdom::VNode::~VNode() {}

WASMDOM_INLINE
const wasmdom::Attrs& wasmdom::VNode::attrs() const { return _data->data.attrs; }

WASMDOM_INLINE
const wasmdom::Props& wasmdom::VNode::props() const { return _data->data.props; }

WASMDOM_INLINE
const wasmdom::Callbacks& wasmdom::VNode::callbacks() const { return _data->data.callbacks; }

WASMDOM_INLINE
const std::string& wasmdom::VNode::sel() const { return _data->sel; }

WASMDOM_INLINE
const std::string& wasmdom::VNode::key() const { return _data->key; }

WASMDOM_INLINE
const std::string& wasmdom::VNode::ns() const { return _data->ns; }

WASMDOM_INLINE
unsigned int wasmdom::VNode::hash() const { return _data->hash; }

WASMDOM_INLINE
int wasmdom::VNode::elm() const { return _data->elm; }

WASMDOM_INLINE
void wasmdom::VNode::setElm(int nodeElm) { _data->elm = nodeElm; }

WASMDOM_INLINE
const wasmdom::Children& wasmdom::VNode::children() const { return _data->children; }

WASMDOM_INLINE
void wasmdom::VNode::normalize() { normalize(false); }

WASMDOM_INLINE
wasmdom::VNode::operator bool() const { return _data != nullptr; }

WASMDOM_INLINE
bool wasmdom::VNode::operator!() const { return _data == nullptr; }

WASMDOM_INLINE
bool wasmdom::VNode::operator==(const VNode& other) const { return _data == other._data; }
