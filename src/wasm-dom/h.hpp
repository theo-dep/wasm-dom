#pragma once

#include <wasm-dom/vnodeforward.hpp>

#include <map>
#include <string>
#include <vector>

namespace wasmdom
{

    struct Data;
    class VNode;

    VNode* h(const std::string& sel);
    VNode* h(const std::string& sel, const std::string& text);
    VNode* h(const std::string& sel, const bool text);
    VNode* h(const std::string& sel, const Data& data);
    VNode* h(const std::string& sel, const Children& children);
    VNode* h(const std::string& sel, VNode* child);
    VNode* h(const std::string& sel, const Data& data, const std::string& text);
    VNode* h(const std::string& sel, const Data& data, const Children& children);
    VNode* h(const std::string& sel, const Data& data, VNode* child);

    inline VNode* t(const std::string& text) { return h(text, true); }

#define SEL(X)                  \
    template <typename... Args> \
    inline VNode* X(Args&&... args) { return h(#X, std::forward<Args>(args)...); }

    SEL(div)
    SEL(span)
    SEL(a)

#undef SEL

}
