#include <wasm-dom.hpp>

#include <emscripten/val.h>

int main()
{
    wasmdom::init();

    auto onClickCallback = [](emscripten::val /*e*/) -> bool {
        emscripten::val::global("console").call<void>("log", std::string("another click"));
        return true;
    };

    // Create the view
    wasmdom::VNode vnode =
        div(("onclick", onClickCallback))(
            { span(("style", "font-weight: normal; font-style: italic"))("This is now italic type"),
              t(" and this is just normal text"),
              div()(
                  a(("style", "cursor: pointer"), ("href", "/"))("I'll take you places!")) });

    // Patch into empty DOM element – this modifies the DOM as a side effect
    wasmdom::VDom vdom(emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root")));
    vdom.patch(vnode);

    return 0;
}
