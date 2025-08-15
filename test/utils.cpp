#include "utils.hpp"

#include "wasm-dom/domapi.hpp"
#include "wasm-dom/vnode.hpp"

#include <emscripten.h>

emscripten::val getRoot()
{
    return emscripten::val::global("document").call<emscripten::val>("getElementById", emscripten::val("root"));
}

emscripten::val getBodyFirstChild()
{
    return emscripten::val::global("document")["body"]["firstChild"];
}

EM_JS(void, createDom, (), {
    const { JSDOM } = require("jsdom");
    const dom = new JSDOM('<!DOCTYPE html><body></body>');
    globalThis.dom = dom;
    globalThis.window = dom.window;
    globalThis.document = dom.window.document;
    globalThis.navigator = dom.window.navigator;
});

void setupDom()
{
    createDom();

    emscripten::val document = emscripten::val::global("document");
    emscripten::val body = document["body"];

    // const root = document.createElement('div');
    emscripten::val root = document.call<emscripten::val>("createElement", std::string("div"));

    // root.setAttribute('id', 'root');
    root.call<void>("setAttribute", std::string("id"), std::string("root"));

    // document.body.appendChild(root);
    body.call<void>("appendChild", root);
}

bool onClick(emscripten::val /*event*/)
{
    return true;
}
