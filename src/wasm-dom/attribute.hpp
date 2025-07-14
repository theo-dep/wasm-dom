#pragma once

#include <emscripten/val.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <variant>

namespace wasmdom
{

    using Callback = std::function<bool(emscripten::val)>;
    using Attribute = std::variant<std::string, emscripten::val, Callback>;

    using Attributes = std::unordered_multimap<std::string, Attribute>;

    using Attrs = std::unordered_map<std::string, std::string>;
    using Props = std::unordered_map<std::string, emscripten::val>;
    using Callbacks = std::unordered_map<std::string, Callback>;

    struct VNodeAttributes
    {
        Attrs attrs;
        Props props;
        Callbacks callbacks;
    };

    VNodeAttributes attributesToVNode(Attributes attributes);

}
