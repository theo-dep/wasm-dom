#include "domrecycler.hpp"

#include "domkeys.hpp"
#include "internals/domfactory.hpp"
#include "internals/domrecyclerfactory.hpp"
#include "internals/utils.hpp"

#include <emscripten/em_js.h>

namespace wasmdom::internals
{
    EM_JS(bool, testGC, (), {
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
    consteval DomFactoryVTable makeDomVTable()
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

wasmdom::DomRecycler::DomRecycler(bool useWasmGC)
    : _factory{ internals::testGC() && useWasmGC ? &internals::domFactoryVTable : &internals::domRecyclerFactoryVTable }
{
}

#ifdef WASMDOM_COVERAGE
wasmdom::DomRecycler::~DomRecycler() = default;
#endif

emscripten::val wasmdom::DomRecycler::create(const std::string& name)
{
    return _factory->create(*this, name);
}

emscripten::val wasmdom::DomRecycler::createNS(const std::string& name, const std::string& ns)
{
    return _factory->createNS(*this, name, ns);
}

emscripten::val wasmdom::DomRecycler::createText(const std::string& text)
{
    return _factory->createText(*this, text);
}

emscripten::val wasmdom::DomRecycler::createComment(const std::string& comment)
{
    return _factory->createComment(*this, comment);
}

void wasmdom::DomRecycler::collect(emscripten::val node)
{
    _factory->collect(*this, node);
}

std::vector<emscripten::val> wasmdom::DomRecycler::nodes(const std::string& name) const
{
    const auto nodeIt = _nodes.find(name);
    if (nodeIt != _nodes.cend())
        return nodeIt->second;
    return {};
}
