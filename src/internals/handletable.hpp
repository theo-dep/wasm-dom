#pragma once

#include "internals/jsapi.hpp"

#include <emscripten/val.h>

#include <cstdint>

namespace wasmdom::internals
{
    using NodeId = std::uint32_t;
    inline constexpr NodeId nullNodeId = 0;

    inline NodeId allocNode(const emscripten::val& v)
    {
        if (v.isNull() || v.isUndefined())
            return nullNodeId;
        return jsapi::wdom_alloc(v.as_handle());
    }

    inline emscripten::val resolveNode(NodeId id)
    {
        return emscripten::val::take_ownership(jsapi::wdom_get(id));
    }

    inline void retainNode(NodeId id)
    {
        if (id != nullNodeId)
            jsapi::wdom_retain(id);
    }

    inline void dropNode(NodeId id)
    {
        if (id != nullNodeId)
            jsapi::wdom_drop(id);
    }
}
