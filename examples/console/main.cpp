#include <wasm-dom.hpp>

#include <emscripten/val.h>

int main()
{
    wasmdom::init();

    // Create the view
    using namespace wasmdom;
    wasmdom::VNode vnode = {
        div(
            Callbacks{ { "onclick",
                         [](emscripten::val /*e*/) -> bool {
                             emscripten::val::global("console").call<void>("log", emscripten::val("another click"));
                             return true;
                         } } },
            Children{ span(Attrs{ { "style", "font-weight: normal; font-style: italic" } },
                           "This is now italic type"),
                      t(" and this is just normal text"),
                      div(
                          a(Attrs{ { "href", "/" } }, "I'll take you places!")) })
    };

    // Patch into empty DOM element – this modifies the DOM as a side effect
    wasmdom::VDom vdom(emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root")));
    vdom.patch(vnode);

    return 0;
}
