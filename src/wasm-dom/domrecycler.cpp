#include "domrecycler.hpp"

#include <emscripten.h>

#include <algorithm>
#include <ranges>
#include <unordered_map>

wasmdom::DomRecycler& wasmdom::recycler()
{
    static DomRecycler recycler(true);
    return recycler;
}

namespace wasmdom
{
    EM_JS(bool, testGC, (), {
        // https://github.com/GoogleChromeLabs/wasm-feature-detect/blob/main/src/detectors/gc/index.js
        return WebAssembly.validate(new Uint8Array([0, 97, 115, 109, 1, 0, 0, 0, 1, 5, 1, 95, 1, 120, 0]));
    })

    std::string upper(const std::string& str)
    {
        static const auto toupper{
            [](const std::string::value_type& c) -> std::string::value_type {
                return static_cast<std::string::value_type>(std::toupper(c));
            }
        };
        std::string upperStr = str;
        std::ranges::copy(std::views::transform(str, toupper), upperStr.begin());
        return upperStr;
    }

    struct DomFactory
    {
        static emscripten::val create(DomRecycler&, const std::string& name)
        {
            return emscripten::val::global("document").call<emscripten::val>("createElement", name);
        }
        static emscripten::val createNS(DomRecycler&, const std::string& name, const std::string& ns)
        {
            emscripten::val node = emscripten::val::global("document").call<emscripten::val>("createElementNS", ns, name);
            node.set("asmDomNS", ns);
            return node;
        }
        static emscripten::val createText(DomRecycler&, const std::string& text)
        {
            return emscripten::val::global("document").call<emscripten::val>("createTextNode", text);
        }
        static emscripten::val createComment(DomRecycler&, const std::string& comment)
        {
            return emscripten::val::global("document").call<emscripten::val>("createComment", comment);
        }

        static void collect(DomRecycler&, emscripten::val /*node*/) {} // // LCOV_EXCL_LINE
    };

    struct DomRecyclerFactory
    {
        static emscripten::val create(DomRecycler& recycler, const std::string& name);
        static emscripten::val createNS(DomRecycler& recycler, const std::string& name, const std::string& ns);
        static emscripten::val createText(DomRecycler& recycler, const std::string& text);
        static emscripten::val createComment(DomRecycler& recycler, const std::string& comment);

        static void collect(DomRecycler& recycler, emscripten::val node);
    };

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

    static inline constexpr DomFactoryVTable domFactoryVTable = makeDomVTable<DomFactory>();
    static inline constexpr DomFactoryVTable domRecyclerFactoryVTable = makeDomVTable<DomRecyclerFactory>();
}

wasmdom::DomRecycler::DomRecycler(bool useWasmGC)
    : _factory{ testGC() && useWasmGC ? &domFactoryVTable : &domRecyclerFactoryVTable }
{
}

#ifdef WASMDOM_COVERAGE
wasmdom::DomRecycler::~DomRecycler() = default;
#endif

emscripten::val wasmdom::DomRecyclerFactory::create(DomRecycler& recycler, const std::string& name)
{
    std::vector<emscripten::val>& list = recycler._nodes[upper(name)];

    if (list.empty())
        return DomFactory::create(recycler, name);

    emscripten::val node = list.back();
    list.pop_back();
    return node;
}

emscripten::val wasmdom::DomRecyclerFactory::createNS(DomRecycler& recycler, const std::string& name, const std::string& ns)
{
    std::vector<emscripten::val>& list = recycler._nodes[upper(name) + ns];

    if (list.empty())
        return DomFactory::createNS(recycler, name, ns);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("asmDomNS", ns);
    return node;
}

emscripten::val wasmdom::DomRecyclerFactory::createText(DomRecycler& recycler, const std::string& text)
{
    constexpr const char* textKey = "#TEXT";
    std::vector<emscripten::val>& list = recycler._nodes[textKey];

    if (list.empty())
        return DomFactory::createText(recycler, text);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", text);
    return node;
}

emscripten::val wasmdom::DomRecyclerFactory::createComment(DomRecycler& recycler, const std::string& comment)
{
    constexpr const char* commentKey = "#COMMENT";
    std::vector<emscripten::val>& list = recycler._nodes[commentKey];

    if (list.empty())
        return DomFactory::createComment(recycler, comment);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", comment);
    return node;
}

void wasmdom::DomRecyclerFactory::collect(DomRecycler& recycler, emscripten::val node)
{
    // clean
    for (emscripten::val child = node["lastChild"]; !child.isNull(); child = node["lastChild"]) {
        node.call<void>("removeChild", child);
        collect(recycler, child);
    }

    if (!node["attributes"].isUndefined()) {
        for (int i : std::views::iota(0, node["attributes"]["length"].as<int>())) {
            node.call<void>("removeAttribute", node["attributes"][i]["name"]);
        }
    }

    node.set("asmDomVNodeCallbacks", emscripten::val::undefined());

    if (!node["asmDomRaws"].isUndefined()) {
        for (int i : std::views::iota(0, node["asmDomRaws"]["length"].as<int>())) {
            node.set(node["asmDomRaws"][i], emscripten::val::undefined());
        }
        node.set("asmDomRaws", emscripten::val::undefined());
    }

    if (!node["asmDomEvents"].isUndefined()) {
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", node["asmDomEvents"]);
        for (int i : std::views::iota(0, keys["length"].as<int>())) {
            emscripten::val event = keys[i];
            node.call<void>("removeEventListener", event, node["asmDomEvents"][event], false);
        }
        node.set("asmDomEvents", emscripten::val::undefined());
    }

    if (!node["nodeValue"].isNull() && !node["nodeValue"].as<std::string>().empty()) {
        node.set("nodeValue", std::string{});
    }

    emscripten::val nodeKeys = emscripten::val::global("Object").call<emscripten::val>("keys", node);
    for (int i : std::views::iota(0, nodeKeys["length"].as<int>())) {
        std::string key = nodeKeys[i].as<std::string>();
        if (!key.starts_with("asmDom")) {
            node.set(key, emscripten::val::undefined());
        }
    }

    // collect
    std::string nodeName = upper(node["nodeName"].as<std::string>());
    if (!node["asmDomNS"].isUndefined()) {
        nodeName += node["namespaceURI"].as<std::string>();
    }

    std::vector<emscripten::val>& list = recycler._nodes[nodeName];
    list.push_back(node);
}

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
