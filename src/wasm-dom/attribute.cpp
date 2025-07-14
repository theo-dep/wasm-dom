#include "attribute.hpp"

template <class... Ts>
struct overloaded : Ts...
{
    using Ts::operator()...;
};

wasmdom::VNodeAttributes wasmdom::attributesToVNode(Attributes attributes)
{
    wasmdom::VNodeAttributes vnodeAttributes;
    for (const auto& [key, val] : attributes) {
        std::visit(overloaded{
                       [&](const std::string& attr) { vnodeAttributes.attrs.emplace(key, attr); },
                       [&](const emscripten::val& prop) { vnodeAttributes.props.emplace(key, prop); },
                       [&](const Callback& callback) { vnodeAttributes.callbacks.emplace(key, callback); } },
                   val);
    }
    return vnodeAttributes;
}
