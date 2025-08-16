#include "jsdom.hpp"

#include <emscripten.h>

// call in JS side, we compile WASM without filesystem
EM_JS(emscripten::EM_VAL, requireJSDom, (), { return Emval.toHandle(require("jsdom")); })

JSDom::JSDom()
{
    static const emscripten::val jsdom = emscripten::val::take_ownership(requireJSDom());
    _dom = jsdom["JSDOM"].new_(std::string("<!DOCTYPE html><body></body>"));

    emscripten::val globalThis = emscripten::val::global("globalThis");
    globalThis.set("document", document());

    const emscripten::val root = document().call<emscripten::val>("createElement", std::string("div"));
    root.call<void>("setAttribute", std::string("id"), std::string("root"));

    const emscripten::val body = document()["body"];
    body.call<void>("appendChild", root);
}

emscripten::val JSDom::document() const
{
    return _dom["window"]["document"];
}

emscripten::val JSDom::root() const
{
    return document().call<emscripten::val>("getElementById", emscripten::val("root"));
}

emscripten::val JSDom::bodyFirstChild() const
{
    return document()["body"]["firstChild"];
}
