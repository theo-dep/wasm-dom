#include "h.hpp"

#include "vnode.hpp"

wasmdom::VNode* wasmdom::h(std::string sel)
{
    return new VNode(sel);
}

wasmdom::VNode* wasmdom::h(std::string sel, std::string text)
{
    return new VNode(sel, text);
}

wasmdom::VNode* wasmdom::h(std::string sel, bool text)
{
    return new VNode(sel, text);
}

wasmdom::VNode* wasmdom::h(std::string sel, Attributes attrs)
{
    return new VNode(sel, attrs);
}

wasmdom::VNode* wasmdom::h(std::string sel, Children children)
{
    return new VNode(sel, children);
}

wasmdom::VNode* wasmdom::h(std::string sel, std::initializer_list<VNode*> children)
{
    return new VNode(sel, children);
}

wasmdom::VNode* wasmdom::h(std::string sel, VNode* child)
{
    return new VNode(sel, child);
}

wasmdom::VNode* wasmdom::h(std::string sel, Attributes attrs, std::string text)
{
    return new VNode(sel, attrs, text);
}

wasmdom::VNode* wasmdom::h(std::string sel, Attributes attrs, Children children)
{
    return new VNode(sel, attrs, children);
}

wasmdom::VNode* wasmdom::h(std::string sel, Attributes attrs, std::initializer_list<VNode*> children)
{
    return new VNode(sel, attrs, children);
}

wasmdom::VNode* wasmdom::h(std::string sel, Attributes attrs, VNode* child)
{
    return new VNode(sel, attrs, child);
}

#define SEL_IMPL(X)                                                                                                         \
    wasmdom::VNode* wasmdom::X() { return h(#X); }                                                                          \
    wasmdom::VNode* wasmdom::X(std::string text) { return h(#X, text); }                                                    \
    wasmdom::VNode* wasmdom::X(bool text) { return h(#X, text); }                                                           \
    wasmdom::VNode* wasmdom::X(Attributes attrs) { return h(#X, attrs); }                                                   \
    wasmdom::VNode* wasmdom::X(Children children) { return h(#X, children); }                                               \
    wasmdom::VNode* wasmdom::X(std::initializer_list<VNode*> children) { return h(#X, children); }                          \
    wasmdom::VNode* wasmdom::X(VNode* child) { return h(#X, child); }                                                       \
    wasmdom::VNode* wasmdom::X(Attributes attrs, std::string text) { return h(#X, attrs, text); }                           \
    wasmdom::VNode* wasmdom::X(Attributes attrs, Children children) { return h(#X, attrs, children); }                      \
    wasmdom::VNode* wasmdom::X(Attributes attrs, std::initializer_list<VNode*> children) { return h(#X, attrs, children); } \
    wasmdom::VNode* wasmdom::X(Attributes attrs, VNode* child) { return h(#X, attrs, child); }

SEL_IMPL(div)
SEL_IMPL(span)
SEL_IMPL(a)

#undef SEL_IMPL
