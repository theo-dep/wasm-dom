#include <wasm-dom.hpp>

#include <emscripten.h>
#include <emscripten/val.h>

#include <string>

EM_JS(void, createHelloComponent, (), {
    class HelloComponent extends HTMLElement
    {
        static get observedAttributes()
        {
            return ['name'];
        }

        get props()
        {
            return {
                name : this.name,
            };
        }

        get name()
        {
            return this.getAttribute('name') || 'World';
        }

        set name(val)
        {
            this.setAttribute('name', val);
        }

        constructor()
        {
            super();
            this.attachShadow({ mode : 'open' });
            this.render();
        }

        attributeChangedCallback()
        {
            this.render();
            this.shadowRoot.dispatchEvent(new Event('change', { bubbles : true, composed : true }));
        }

        render()
        {
            const name = this.props.name;
            this.shadowRoot.textContent = `Hello ${name}!`;
        }
    }

    customElements.define('hello-component', HelloComponent);
})

int main()
{
    createHelloComponent();
    wasmdom::init();

    auto onChangeCallback = [](emscripten::val e) -> bool {
        emscripten::val::global("console").call<void>("log", emscripten::val("name changed:"), e["target"]["name"]);
        return true;
    };

    using namespace wasmdom::dsl;

    wasmdom::VNode header =
        div()(
            { t("Here is an \"Hello\" component that accepts a \"name\" attribute, and emit a \"change\" event, please open the console"),
              br(),
              br() }
        );

    wasmdom::VNode oldVnode =
        div()(
            { header,
              wasmdom::VNode(
                  "hello-component",
                  ("name", "world"s),
                  ("onchange", onChangeCallback)
              ) }
        );

    wasmdom::VDom vdom(
        emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
    );
    vdom.patch(oldVnode);

    wasmdom::VNode newVnode =
        div()(
            { header,
              wasmdom::VNode(
                  "hello-component",
                  ("name", "wasm-dom"s),
                  ("onchange", onChangeCallback)
              ) }
        );

    vdom.patch(newVnode);

    return 0;
};
