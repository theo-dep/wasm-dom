#include <wasm-dom.hpp>
using namespace wasmdom::dsl;

#include <emscripten/val.h>

int main()
{
    wasmdom::init();

    auto onClickCallback = [](emscripten::val /*e*/) -> bool {
        emscripten::val::global("console").call<void>("log", "another click"s);
        return true;
    };

    // Create the view
    wasmdom::VNode vnode =
        div(("onclick", onClickCallback))(
            { span(("style", "font-weight: normal; font-style: italic"s))("This is now italic type"),
              t(" and this is just normal text"),
              div()(
                  a(("style", "cursor: pointer"s), ("href", "/"s))("I'll take you places!")) });

    // Patch into empty DOM element – this modifies the DOM as a side effect
    wasmdom::VDom vdom(emscripten::val::global("document").call<emscripten::val>("getElementById", "root"s));
    vdom.patch(vnode);

    return 0;
}
