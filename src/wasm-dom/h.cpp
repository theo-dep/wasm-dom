#include "h.hpp"

#include "vnode.hpp"

wasmdom::VNode wasmdom::h(const std::string& sel)
{
    return VNode(sel);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const std::string& text)
{
    return VNode(sel, text);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const bool text)
{
    return VNode(sel, text);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const Data& data)
{
    return VNode(sel, data);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const Children& children)
{
    return VNode(sel, children);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const VNode& child)
{
    return VNode(sel, child);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const Data& data, const std::string& text)
{
    return VNode(sel, data, text);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const Data& data, const Children& children)
{
    return VNode(sel, data, children);
}

wasmdom::VNode wasmdom::h(const std::string& sel, const Data& data, const VNode& child)
{
    return VNode(sel, data, child);
}
