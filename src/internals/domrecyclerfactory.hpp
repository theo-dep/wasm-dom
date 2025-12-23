#pragma once

#include "internals/domfactory.hpp"
#include "internals/domrecycler.hpp"
#include "internals/jsapi.hpp"
#include "internals/utils.hpp"

#include <emscripten/val.h>

#include <ranges>

namespace wasmdom::internals
{
    struct DomRecyclerFactory
    {
        static emscripten::val create(DomRecycler& recycler, const std::string& name);
        static emscripten::val createNS(DomRecycler& recycler, const std::string& name, const std::string& ns);
        static emscripten::val createText(DomRecycler& recycler, const std::string& text);
        static emscripten::val createComment(DomRecycler& recycler, const std::string& comment);

        static void collect(DomRecycler& recycler, emscripten::val node);
    };
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::create(DomRecycler& recycler, const std::string& name)
{
    std::vector<emscripten::val>& list = recycler._nodes[upper(name)];

    if (list.empty())
        return DomFactory::create(recycler, name);

    emscripten::val node = list.back();
    list.pop_back();
    return node;
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::createNS(DomRecycler& recycler, const std::string& name, const std::string& ns)
{
    std::vector<emscripten::val>& list = recycler._nodes[upper(name) + ns];

    if (list.empty())
        return DomFactory::createNS(recycler, name, ns);

    emscripten::val node = list.back();
    list.pop_back();

    node.set(nodeNSKey, ns);
    return node;
}

inline emscripten::val wasmdom::internals::DomRecyclerFactory::createText(DomRecycler& recycler, const std::string& text)
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

inline emscripten::val wasmdom::internals::DomRecyclerFactory::createComment(DomRecycler& recycler, const std::string& comment)
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

inline void wasmdom::internals::DomRecyclerFactory::collect(DomRecycler& recycler, emscripten::val node)
{
    // clean
    for (emscripten::val child = node["lastChild"]; !child.isNull(); child = node["lastChild"]) {
        jsapi::removeChild(node.as_handle(), child.as_handle());
        collect(recycler, child);
    }

    if (!node["attributes"].isUndefined()) {
        for (int i : std::views::iota(0, node["attributes"]["length"].as<int>())) {
            jsapi::removeAttribute(node.as_handle(), node["attributes"][i]["name"].as<std::string>().c_str());
        }
    }

    if (!node[nodeRawsKey].isUndefined()) {
        for (int i : std::views::iota(0, node[nodeRawsKey]["length"].as<int>())) {
            node.set(node[nodeRawsKey][i], emscripten::val::undefined());
        }
        node.set(nodeRawsKey, emscripten::val::undefined());
    }

    static const emscripten::val objectEntries = emscripten::val::global("Object")["entries"];

    if (!node[nodeEventsKey].isUndefined()) {
        const emscripten::val entries = objectEntries(node[nodeEventsKey]);
        for (int i : std::views::iota(0, entries["length"].as<int>())) {
            const emscripten::val pair = entries[i];
            const emscripten::val event = pair[0];
            const emscripten::val eventCallback = pair[1];
            jsapi::removeEventListener_(node.as_handle(), event.as<std::string>().c_str(), eventCallback.as_handle());
        }
        node.set(nodeEventsKey, emscripten::val::undefined());
    }

    if (!node["nodeValue"].isNull() && node["nodeValue"].isString()) {
        node.set("nodeValue", std::string{});
    }

    static const emscripten::val objectKeys = emscripten::val::global("Object")["keys"];

    const emscripten::val nodeKeys = objectKeys(node);
    for (int i : std::views::iota(0, nodeKeys["length"].as<int>())) {
        const std::string key = nodeKeys[i].as<std::string>();
        if (!key.starts_with(nodeKeyPrefix)) {
            node.set(key, emscripten::val::undefined());
        }
    }

    // collect
    std::string nodeName = upper(node["nodeName"].as<std::string>());
    if (!node[nodeNSKey].isUndefined()) {
        nodeName += node["namespaceURI"].as<std::string>();
    }

    std::vector<emscripten::val>& list = recycler._nodes[nodeName];
    list.push_back(node);
}
