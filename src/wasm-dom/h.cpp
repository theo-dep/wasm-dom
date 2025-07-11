#include "h.hpp"

#include "vnode.hpp"

#include <map>
#include <string>
#include <vector>

wasmdom::VNode* wasmdom::h(const std::string& sel)
{
    return new VNode(sel);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const std::string& text)
{
    return new VNode(sel, text);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const bool text)
{
    return new VNode(sel, text);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const Data& data)
{
    return new VNode(sel, data);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const Children& children)
{
    return new VNode(sel, children);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, VNode* child)
{
    return new VNode(sel, child);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const Data& data, const std::string& text)
{
    return new VNode(sel, data, text);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const Data& data, const Children& children)
{
    return new VNode(sel, data, children);
}

wasmdom::VNode* wasmdom::h(const std::string& sel, const Data& data, VNode* child)
{
    return new VNode(sel, data, child);
}
