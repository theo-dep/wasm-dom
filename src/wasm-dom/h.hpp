#pragma once

#include "attribute.hpp"
#include "vnodeforward.hpp"

#include <string>
#include <vector>

namespace wasmdom
{

    VNode* h(std::string sel);
    VNode* h(std::string sel, std::string text);
    VNode* h(std::string sel, bool text);
    VNode* h(std::string sel, Attributes attrs);
    VNode* h(std::string sel, Children children);
    VNode* h(std::string sel, std::initializer_list<VNode*> children);
    VNode* h(std::string sel, VNode* child);
    VNode* h(std::string sel, Attributes attrs, std::string text);
    VNode* h(std::string sel, Attributes attrs, Children children);
    VNode* h(std::string sel, Attributes attrs, std::initializer_list<VNode*> children);
    VNode* h(std::string sel, Attributes attrs, VNode* child);

    inline VNode* t(std::string text) { return h(text, true); }

#define SEL(X)                                                          \
    VNode* X();                                                         \
    VNode* X(std::string text);                                         \
    VNode* X(bool text);                                                \
    VNode* X(Attributes attrs);                                         \
    VNode* X(Children children);                                        \
    VNode* X(std::initializer_list<VNode*> children);                   \
    VNode* X(VNode* child);                                             \
    VNode* X(Attributes attrs, std::string text);                       \
    VNode* X(Attributes attrs, Children children);                      \
    VNode* X(Attributes attrs, std::initializer_list<VNode*> children); \
    VNode* X(Attributes attrs, VNode* child);

    SEL(div)
    SEL(span)
    SEL(a)

#undef SEL

}
