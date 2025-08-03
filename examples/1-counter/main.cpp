#include <wasm-dom.hpp>

#include <emscripten/val.h>

#include <memory>

void render();
std::unique_ptr<wasmdom::VDom> vdom = nullptr;

int counter = 1;

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
    vdom->patch(newNode);
};

int main()
{
    wasmdom::init();

    vdom = std::make_unique<wasmdom::VDom>(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
    );

    render();

    return 0;
};
