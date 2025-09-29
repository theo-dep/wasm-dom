#include <wasm-dom.hpp>

#include <emscripten/val.h>

wasmdom::VDom vdom;

int counter = 1;

void render();

bool decrease(emscripten::val)
{
    counter--;
    render();
    return true;
}

bool increase(emscripten::val)
{
    counter++;
    render();
    return true;
}

void render()
{
    using namespace wasmdom::dsl;
    wasmdom::VNode newNode =
        div()({
            a(("class", "button"s), ("onclick", f(decrease)))("-"),
            t(std::to_string(counter)),
            a(("class", "button"s), ("onclick", f(increase)))("+"),
        });
    vdom.patch(newNode);
};

int main()
{
    vdom = wasmdom::VDom(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
    );

    render();

    return 0;
};
