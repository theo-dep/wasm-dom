#pragma once

#include <wasm-dom/vnodeforward.hpp>

#include <map>
#include <string>
#include <vector>

namespace wasmdom
{

    struct Data;
    struct VNode;

    VNode* h(const std::string& sel);
    VNode* h(const std::string& sel, const std::string& text);
    VNode* h(const std::string& sel, const bool text);
    VNode* h(const std::string& sel, const Data& data);
    VNode* h(const std::string& sel, const Children& children);
    VNode* h(const std::string& sel, VNode* child);
    VNode* h(const std::string& sel, const Data& data, const std::string& text);
    VNode* h(const std::string& sel, const Data& data, const Children& children);
    VNode* h(const std::string& sel, const Data& data, VNode* child);

}
