#include <wasm-dom.hpp>

#include <emscripten/em_js.h>

using namespace wasmdom;
using namespace wasmdom::dsl;

// Inspired from https://react.dev/learn/synchronizing-with-effects#challenges

// This is made for demonstration purpose. A good property binding library like KDBindings will be better.
template <typename T>
class Property
{
    T _val;
    std::function<void(T)> _binding;

public:
    template <typename Fn>
    void bind(Fn f) { _binding = f; }

    template <typename U>
    Property(U val)
        : _val(val)
    {
    }

    template <typename U>
    Property& operator=(U val)
    {
        _val = val;
        if (_binding)
            _binding(val);
        return *this;
    }

    operator T() const { return _val; }
};

Property<bool> show = false;
Property<std::string> name = "Taylor";

VDom vdom;

EM_JS(void, focus, (emscripten::EM_VAL handle), {
    const node = Emval.toValue(handle);
    const nodeValue = node.value;
    node.value = "";

    node.focus();

    // https://stackoverflow.com/questions/511088/use-javascript-to-place-cursor-at-end-of-text-in-text-input-element
    node.value = nodeValue;
});

void render()
{
    VNode vnode = div(("id", "root"s), ("style", "width: 50%; height: 100%"s))(
        { button(
              ("style", "font-size: 100%; width: 30%; margin-top: 25%"s),
              ("onclick",
               [](emscripten::val) {
                   show = !show;
                   return true;
               })
          )(
              (show ? "Hide" : "Show") + " form"s
          ),
          br(),
          hr(),
          (show
               ? fragment()(
                     { label(("for", "example"s))("Enter your name: "),
                       input(
                           ("style", "font-size: 100%"s),
                           ("id", "example"s), ("value", name),
                           ("oninput",
                            [](emscripten::val e) {
                                name = e["target"]["value"].as<std::string>();
                                return true;
                            }),
                           (onMount,
                            [](emscripten::val e) {
                                focus(e.as_handle());
                            })
                       ),
                       p()(
                           { t("Hello, "), b()(name) }
                       ) }
                 )
               : fragment()) }
    );

    vdom.patch(vnode);
}

int main()
{
    vdom = VDom(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
    );

    show.bind([](bool) { render(); });
    // an improvement would be to bind the property (aka ref) to nodes to change directly the text
    // but wasm-dom only loop over nodes to change the text.
    name.bind([](std::string) { render(); });

    render();
}
