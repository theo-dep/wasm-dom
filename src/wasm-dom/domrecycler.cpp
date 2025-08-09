#include "domrecycler.hpp"

#include <algorithm>
#include <ranges>

wasmdom::DomRecycler& wasmdom::recycler()
{
    static DomRecycler recycler;
    return recycler;
}

namespace wasmdom
{
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
}

emscripten::val wasmdom::DomRecycler::create(const std::string& name)
{
    std::vector<emscripten::val>& list = _nodes[upper(name)];

    if (list.empty())
        return emscripten::val::global("document").call<emscripten::val>("createElement", name);

    emscripten::val node = list.back();
    list.pop_back();
    return node;
}

emscripten::val wasmdom::DomRecycler::createNS(const std::string& name, const std::string& ns)
{
    std::vector<emscripten::val>& list = _nodes[upper(name) + ns];

    emscripten::val node;
    if (list.empty()) {
        node = emscripten::val::global("document").call<emscripten::val>("createElementNS", ns, name);
    } else {
        node = list.back();
        list.pop_back();
    }

    node.set("asmDomNS", ns);
    return node;
}

emscripten::val wasmdom::DomRecycler::createText(const std::string& text)
{
    constexpr const char* textKey = "#TEXT";
    std::vector<emscripten::val>& list = _nodes[textKey];

    if (list.empty())
        return emscripten::val::global("document").call<emscripten::val>("createTextNode", text);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", text);
    return node;
}

emscripten::val wasmdom::DomRecycler::createComment(const std::string& comment)
{
    constexpr const char* commentKey = "#COMMENT";
    std::vector<emscripten::val>& list = _nodes[commentKey];

    if (list.empty())
        return emscripten::val::global("document").call<emscripten::val>("createComment", comment);

    emscripten::val node = list.back();
    list.pop_back();

    node.set("nodeValue", comment);
    return node;
}

void wasmdom::DomRecycler::collect(emscripten::val node)
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

std::vector<emscripten::val> wasmdom::DomRecycler::nodes(const std::string& name) const
{
    if (_nodes.contains(name))
        return _nodes.at(name);
    return {};
}
