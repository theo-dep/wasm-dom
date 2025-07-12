#include "utils.hpp"

#include "wasm-dom/config.hpp"
#include "wasm-dom/patch.hpp"
#include "wasm-dom/vnode.hpp"

emscripten::val getRoot()
{
    return emscripten::val::global("document").call<emscripten::val>("getElementById", emscripten::val("root"));
}

emscripten::val getBodyFirstChild()
{
    return emscripten::val::global("document")["body"]["firstChild"];
}

emscripten::val getNode(wasmdom::VNode* vnode)
{
    return emscripten::val::module_property("nodes")[std::to_string(vnode->elm).c_str()];
}

void setupDom()
{
    emscripten::val document = emscripten::val::global("document");
    emscripten::val body = document["body"];

    // while (body.firstChild) body.removeChild(body.firstChild);
    while (!body["firstChild"].isNull()) {
        body.call<emscripten::val>("removeChild", body["firstChild"]);
    }

    // const root = document.createElement('div');
    emscripten::val root = document.call<emscripten::val>("createElement", std::string("div"));

    // root.setAttribute('id', 'root');
    root.call<void>("setAttribute", std::string("id"), std::string("root"));

    // document.body.appendChild(root);
    body.call<void>("appendChild", root);
}

void reset()
{
    wasmdom::currentNode = NULL;
    wasmdom::CLEAR_MEMORY = true;
    wasmdom::UNSAFE_PATCH = false;
}

bool onClick(emscripten::val event)
{
    return true;
}
