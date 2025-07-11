#include <wasm-dom.hpp>

#include <emscripten/val.h>

int main()
{
    wasmdom::Config config = wasmdom::Config();
    wasmdom::init(config);

    // Create the view
    wasmdom::VNode* vnode = {
        wasmdom::h("div",
                   wasmdom::Data(
                       wasmdom::Callbacks{
                           { "onclick", [](emscripten::val e) -> bool {
                                emscripten::val::global("console").call<void>("log", emscripten::val("another click"));
                                return true;
                            } } }),
                   wasmdom::Children{                                                                                                                                                      //
                                      wasmdom::h("span", wasmdom::Data(wasmdom::Attrs{ { "style", "font-weight: normal; font-style: italic" } }), std::string("This is now italic type")), //
                                      wasmdom::h(" and this is just normal text", true),                                                                                                   //
                                      wasmdom::h("div",                                                                                                                                    //
                                                 wasmdom::Children{                                                                                                                        //
                                                                    wasmdom::h("a", wasmdom::Data(wasmdom::Attrs{ { "href", "/" } }), std::string("I'll take you places!")) }) })
    };

    // Patch into empty DOM element – this modifies the DOM as a side effect
    wasmdom::patch(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root")),
        vnode);

    return 0;
}
