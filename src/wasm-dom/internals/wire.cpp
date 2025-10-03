#include "wire.hpp"

#include "wasm-dom/internals/conf.h"
#include "wasm-dom/internals/domkeys.hpp"

#include <emscripten/bind.h>

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::toJsCallback(const Callback& callback)
{
    return emscripten::val(callback);
}

// Callback binding template specialization, address are owned by VNode
// see https://github.com/emscripten-core/emscripten/issues/25399
namespace emscripten::internal
{
    template <>
    struct BindingType<wasmdom::Callback>
    {
        using WireType = const wasmdom::Callback*;

        static inline WireType toWireType(const wasmdom::Callback& c, rvp::default_tag)
        {
            return &c;
        }

        static inline const wasmdom::Callback& fromWireType(WireType wt, rvp::default_tag)
        {
            return *wt;
        }
    };
}

namespace wasmdom::internals
{
    // in single header mode, the binding function must be registered only once
    // see https://github.com/emscripten-core/emscripten/issues/25219
    __attribute__((weak)) emscripten::internal::InitFunc wasmdomInitEventProxyFunc([] {
        emscripten::class_<Callback>(nodeCallbackName)
            .constructor<>()
            .function("opcall", &Callback::operator());
    });
}
