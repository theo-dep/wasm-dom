#include "domrecycler.hpp"

#include "bind.h"
#include "domfactory.hpp"
#include "domkeys.hpp"
#include "domrecyclerfactory.hpp"
#include "utils.hpp"

#include <wasm-dom/conf.h>

#include <emscripten/em_js.h>
#include <emscripten/val.h>

namespace wasmdom::internals
{
    WASMDOM_EM_JS(bool, testGC, (), {
        // https://github.com/GoogleChromeLabs/wasm-feature-detect/blob/main/src/detectors/gc/index.js
        return WebAssembly.validate(new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0, 1, 5, 1, 95, 1, 120, 0]));
    })

    struct DomFactoryVTable
    {
        emscripten::val (*create)(DomRecycler&, const std::string&) = nullptr;
        emscripten::val (*createNS)(DomRecycler&, const std::string&, const std::string&) = nullptr;
        emscripten::val (*createText)(DomRecycler&, const std::string&) = nullptr;
        emscripten::val (*createComment)(DomRecycler&, const std::string&) = nullptr;
        void (*collect)(DomRecycler&, emscripten::val) = nullptr;
    };

    template <typename T>
    inline consteval DomFactoryVTable makeDomVTable()
    {
        return {
            &T::create,
            &T::createNS,
            &T::createText,
            &T::createComment,
            &T::collect
        };
    }

    static inline constexpr DomFactoryVTable domFactoryVTable = makeDomVTable<internals::DomFactory>();
    static inline constexpr DomFactoryVTable domRecyclerFactoryVTable = makeDomVTable<internals::DomRecyclerFactory>();
}

WASMDOM_SH_INLINE
wasmdom::internals::DomRecycler::DomRecycler(bool useWasmGC)
    : _factory{ testGC() && useWasmGC ? &domFactoryVTable : &domRecyclerFactoryVTable }
{
}

#ifdef WASMDOM_COVERAGE
wasmdom::internals::DomRecycler::~DomRecycler() = default;
#endif

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::DomRecycler::create(const std::string& name)
{
    return _factory->create(*this, name);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::DomRecycler::createNS(const std::string& name, const std::string& ns)
{
    return _factory->createNS(*this, name, ns);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::DomRecycler::createText(const std::string& text)
{
    return _factory->createText(*this, text);
}

WASMDOM_SH_INLINE
emscripten::val wasmdom::internals::DomRecycler::createComment(const std::string& comment)
{
    return _factory->createComment(*this, comment);
}

WASMDOM_SH_INLINE
void wasmdom::internals::DomRecycler::collect(emscripten::val node)
{
    _factory->collect(*this, node);
}

WASMDOM_SH_INLINE
std::vector<emscripten::val> wasmdom::internals::DomRecycler::nodes(const std::string& name) const
{
    const auto nodeIt = _nodes.find(name);
    if (nodeIt != _nodes.cend())
        return nodeIt->second;
    return {};
}
