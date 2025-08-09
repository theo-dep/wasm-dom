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

    struct DomRecycler::DomFactory
    {
        DomFactory() = default;
        virtual ~DomFactory() = default;

        virtual emscripten::val create(const std::string& name)
        {
            return emscripten::val::global("document").call<emscripten::val>("createElement", name);
        }
        virtual emscripten::val createNS(const std::string& name, const std::string& ns)
        {
            emscripten::val node = emscripten::val::global("document").call<emscripten::val>("createElementNS", ns, name);
            node.set("asmDomNS", ns);
            return node;
        }
        virtual emscripten::val createText(const std::string& text)
        {
            return emscripten::val::global("document").call<emscripten::val>("createTextNode", text);
        }
        virtual emscripten::val createComment(const std::string& comment)
        {
            return emscripten::val::global("document").call<emscripten::val>("createComment", comment);
        }

        virtual void collect(emscripten::val /*node*/) {} // // LCOV_EXCL_LINE
    };

    struct DomRecycler::DomRecyclerFactory : DomRecycler::DomFactory
    {
        DomRecyclerFactory() = default;
        ~DomRecyclerFactory() override = default;

        std::unordered_map<std::string, std::vector<emscripten::val>> _nodes;

        emscripten::val create(const std::string& name) override;
        emscripten::val createNS(const std::string& name, const std::string& ns) override;
        emscripten::val createText(const std::string& text) override;
        emscripten::val createComment(const std::string& comment) override;

        void collect(emscripten::val node) override;
    };
}

wasmdom::DomRecycler::DomRecycler(bool useWasmGC)
    : _d_ptr{ testGC() && useWasmGC ? new DomFactory : new DomRecyclerFactory }
{
}

wasmdom::DomRecycler::~DomRecycler()
{
    delete _d_ptr;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::create(const std::string& name)
{
    std::vector<emscripten::val>& list = _nodes[upper(name)];

    if (list.empty())
        return DomFactory::create(name);

    emscripten::val node = list.back();
    list.pop_back();
    return node;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::createNS(const std::string& name, const std::string& ns)
{
    std::vector<emscripten::val>& list = _nodes[upper(name) + ns];

    if (list.empty())
        return DomFactory::createNS(name, ns);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("asmDomNS", ns);
    return node;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::createText(const std::string& text)
{
    constexpr const char* textKey = "#TEXT";
    std::vector<emscripten::val>& list = _nodes[textKey];

    if (list.empty())
        return DomFactory::createText(text);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", text);
    return node;
}

emscripten::val wasmdom::DomRecycler::DomRecyclerFactory::createComment(const std::string& comment)
{
    constexpr const char* commentKey = "#COMMENT";
    std::vector<emscripten::val>& list = _nodes[commentKey];

    if (list.empty())
        return DomFactory::createComment(comment);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", comment);
    return node;
}

void wasmdom::DomRecycler::DomRecyclerFactory::collect(emscripten::val node)
{
    // clean
    for (emscripten::val child = node["lastChild"]; !child.isNull(); child = node["lastChild"]) {
        node.call<void>("removeChild", child);
        collect(child);
    }

    if (!node["attributes"].isUndefined()) {
        for (int i = node["attributes"]["length"].as<int>() - 1; i >= 0; --i) {
            node.call<void>("removeAttribute", node["attributes"][i]["name"]);
        }
    }

    node.set("asmDomVNodeCallbacks", emscripten::val::undefined());

    if (!node["asmDomRaws"].isUndefined()) {
        for (int i = 0; i < node["asmDomRaws"]["length"].as<int>(); ++i) {
            node.set(node["asmDomRaws"][i], emscripten::val::undefined());
        }
        node.set("asmDomRaws", emscripten::val::undefined());
    }

    if (!node["asmDomEvents"].isUndefined()) {
        emscripten::val keys = emscripten::val::global("Object").call<emscripten::val>("keys", node["asmDomEvents"]);
        for (int i = 0; i < keys["length"].as<int>(); ++i) {
            emscripten::val event = keys[i];
            node.call<void>("removeEventListener", event, node["asmDomEvents"][event], false);
        }
        node.set("asmDomEvents", emscripten::val::undefined());
    }

    if (!node["nodeValue"].isNull() && !node["nodeValue"].as<std::string>().empty()) {
        node.set("nodeValue", std::string{});
    }

    emscripten::val nodeKeys = emscripten::val::global("Object").call<emscripten::val>("keys", node);
    for (int i = 0; i < nodeKeys["length"].as<int>(); ++i) {
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

    std::vector<emscripten::val>& list = _nodes[nodeName];
    list.push_back(node);
}

emscripten::val wasmdom::DomRecycler::create(const std::string& name)
{
    return _d_ptr->create(name);
}

emscripten::val wasmdom::DomRecycler::createNS(const std::string& name, const std::string& ns)
{
    return _d_ptr->createNS(name, ns);
}

emscripten::val wasmdom::DomRecycler::createText(const std::string& text)
{
    return _d_ptr->createText(text);
}

emscripten::val wasmdom::DomRecycler::createComment(const std::string& comment)
{
    return _d_ptr->createComment(comment);
}

void wasmdom::DomRecycler::collect(emscripten::val node)
{
    _d_ptr->collect(node);
}

std::vector<emscripten::val> wasmdom::DomRecycler::nodes(const std::string& name) const
{
    DomRecyclerFactory* ptr{ dynamic_cast<DomRecyclerFactory*>(_d_ptr) };
    if (ptr && ptr->_nodes.contains(name))
        return ptr->_nodes.at(name);
    return {};
}
