<a name="top"></a>
# wasm-dom

![experimental](https://img.shields.io/badge/Stability-experimental-orange)
[![Build with Emscripten](https://img.shields.io/github/actions/workflow/status/theo-dep/wasm-dom/pr.yml?label=Build%20with%20Emscripten)](https://github.com/theo-dep/wasm-dom/actions/workflows/pr.yml)
[![Code Coverage](https://img.shields.io/badge/dynamic/yaml?url=https%3A%2F%2Ftheo-dep.github.io%2Fwasm-dom%2Fcoverage-badge.json&query=%24.coverage&label=Code%20Coverage&color=brightgreen)](https://github.com/theo-dep/wasm-dom/actions/workflows/pr.yml)

A minimal C++23 WebAssembly virtual DOM to build SPA (Single page applications).

## Table of Contents

- [History](#history)
- [Motivation](#motivation)
- [Examples](#examples)
- [Getting started](#getting-started)
- [License](#license)

## History

Initial version of wasm-dom is a fork of [asm-dom](https://github.com/mbasso/asm-dom) version 0.7.0, and there is no intention to keep it compatible with it.

### Changelog (since fork)

- Remove JavaScript side, this version kept only the C++ compatibility.
- Create VDom class and allocate VNode on stack to prepare for a domain-specific language (DSL).
  Less than 50% slower relative to initial code before [PR #3](https://github.com/theo-dep/wasm-dom/pull/3) for patch with addition. Other performances are stable.
- Add a domain-specific language (DSL) with attributes in key-value pairs from namespace `wasmdom::dsl`. Text and children are now added with `operator()`. Remove `h` function.
- A single header in [extra](/extra) folder.
- Remove the `init` method and create DOM API functions and DomRecycler singleton class.

## Motivation

From asm-dom author:

> asm-dom is a minimal WebAssembly virtual DOM to build C++ SPA (Single page applications). You can write an entire SPA in C++ and compile it to WebAssembly (or asmjs as fallback) using [Emscripten](https://emscripten.org/), asm-dom will call DOM APIs for you. This will produce an app that `aims to execute at native speed by taking advantage of common hardware capabilities`, also, you can use your C/C++ code without any change, you don't have to create a binding layer to use it (as we have to do if we want to use a C++ lib from JS). Basically we are creating an app in C++ that calls JavaScript if needed instead of the opposite. You can write only once in C++ and share as much code as possible with desktop/mobile apps and web sites. If you want to learn more about performance, please see [this](https://github.com/mbasso/asm-dom/tree/master/benchmarks).
>
> _How can I structure my application with asm-dom?_
>
> asm-dom is a low-level virtual DOM library. It is unopinionated with regards to how you should structure your application.
>
> _How did you come up with the concept of asm-dom?_
>
> At the beginning asm-dom was born from the idea to test the power of WebAssembly in a common use case that is not gaming, VR, AR or Image / video editing. Unfortunately, at the moment, [GC/DOM Integration](http://webassembly.org/docs/future-features/) is a future feature, so, asm-dom isn't totally developed in wasm. All interactions with the DOM are written in JavaScript. This is a big disadvantage because of the overhead of the binding between JS and WASM, in the future asm-dom will be even more powerful, anyway results are satisfying.

## Examples

The library provides runnable example with a static file server, [emrun](https://emscripten.org/docs/compiling/Running-html-files-with-emrun.html) for example.

```sh
emrun build/examples/0-console/Debug
```

1. [Console example](/examples/0-console/main.cpp)
2. [Counter example](/examples/1-counter/main.cpp)
3. [Routing example](/examples/2-routing/main.cpp)
4. [WebComponent example](/examples/3-webcomponent/main.cpp)

### HTML like syntax

wasm-dom can be used with an HTML-like syntax to make your developer experience even better. The syntax is encapsulated in the `wasmdom::dsl` namespace.

```cpp
VNode vnode =
  div()({
    h1()("My Awesome App!"),
    form(("onsubmit", f(onSubmit)))({
      t("Enter name:"),
      input(
        ("type", "text"s),
        ("onchange", f(onChange)),
        ("value", state["value"])
      )
    })
  });
```

### Server Side Rendering

wasm-dom supports server-side rendering, you can write your server in C++ and run it on Node.js with WebAssembly.

```cpp
VNode vnode = view();
std::string appString = vnode.toHTML();
std::string html =
  "<!DOCTYPE html>"
  "<html>"
    "<head>"
      "<title>My Awesome App</title>"
      "<link rel=\"stylesheet\" href=\"/index.css\" />"
    "</head>"

    "<body>"
      + appString +
    "</body>"

    "<script src=\"/bundle.js\"></script>"
  "</html>";
```

### Inline Example

```c++
#include "wasm-dom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

int main() {
  VNode vnode =
    div(("onclick", [](emscripten::val e) -> bool {
          emscripten::val::global("console").call<void>("log", emscripten::val("clicked"));
          return true;
        })
    )({
        span(("style", "font-weight: bold"s))(
          "This is bold"
        ),
        t(" and this is just normal text"),
        a(("href", "/foo"s))(
          "I'll take you places!"
        )
      }
    );

  // Patch into empty DOM element – this modifies the DOM as a side effect
  VDom vdom(
    emscripten::val::global("document").call<emscripten::val>(
      "getElementById",
      std::string("root")
    )
  );
  vdom.patch(vnode);

  return 0;
}
```

## Getting started

### Installation

The project uses the latest version of Emscripten 4.0.10.

To start using wasm-dom without configuration, consider the single header in the [extra](/extra) folder. Just download and include it!

Otherwise, consider a CMake project with these cases:
  - Add wasm-dom as submodule and use `add_subdirectory`
  - Use CMake [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)

### Memory Management

A `VNode` object holds a state shared by other copied `VNode` instances. There is nothing to delete or handle somewhere in the application. The `VDom` will replace the old node with the new one at each patch.

### Boolean Attributes

To set a boolean attribute, like `readonly`, just pass true or false as a string. wasm-dom will handle it for the application.

```cpp
VNode vnode = input(("type", "text"s), ("readonly", "true"s)); // or ("readonly", "false"s)
```

> [!TIP]
> Notice the use of `s` to transform the literal into `std::string`. The key-value syntax uses the `operator,` but it needs a type.

> [!NOTE]
> An improvement would be to pass directly a C++ type as attribute.

### String Encoding

To render an attribute in UTF-8, just use `emscripten::val::u8string`.

### SVG

SVG just works when using a node function for creating virtual nodes. SVG elements are automatically created with the appropriate namespaces.

```cpp
VNode vnode =
  div()({
    svg(("width", "100"s), ("height", "100"s))({
      circle(
        ("cx", "50"s),
        ("cy", "50"s),
        ("r", "40"s),
        ("stroke", "green"s),
        ("stroke-width", "4"s),
        ("fill", "yellow"s)
      )
    })
  });
```

### Ref

To access directly DOM nodes created by wasm-dom, for example to manage focus, text selection, or integrating with third-party DOM libraries, use ref callbacks. ref is a special callback that takes the DOM node as param and can return true or false unconditionally, this is just for simplicity, to maintain the same signatures of other events. ref is called after the DOM node is mounted, if the ref callback changes or after the DOM node is removed from the DOM tree, in this case the param is `emscripten::val::null()`. Here is an example of the first and the last case.

```cpp
bool refCallback(emscripten::val node) {
  // check if node === null
  if (node.strictlyEquals(emscripten::val::null())) {
    // node unmounted
    // do nothing
  } else {
    // node mounted
    // focus input
    node.call<void>("focus");
  }

  return true;
};

int main() {
  VNode vnode1 =
    div()(
      input(("ref", f(refCallback)))
    );

  VDom vdom(
    emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
  );
  vdom.patch(vnode1);

  VNode vnode2 = div();
  vdom.patch(vnode2);

  return 0;
}
```

ref callback is also invoked if it changes, in the following example wasm-dom will call refCallback after the DOM node is mounted and then anotherRefCallback after the update.

```cpp
VNode vnode1 =
  div()(
    input(("ref", f(refCallback)))
  );

VDom vdom(
  emscripten::val::global("document").call<emscripten::val>("getElementById", std::string("root"))
);
vdom.patch(vnode1);

VNode vnode2 =
  div()(
    input(("ref", f(anotherRefCallback)))
  );

vdom.patch(vnode2);
```

Please note that if the project wants to use a lambda as a ref wasm-dom will call it on every update, so, probably avoid something like this.

```cpp
VNode vnode1 =
  div()(
    input(("ref", [&](emscripten::val node) -> bool {
        if (!node.strictlyEquals(emscripten::val::null())) {
          // node mounted
          // focus input
          node.call<void>("focus");
        }

        return true;
      })
    )
  );
```

> [!TIP]
> The use of the function `f` is always necessary in case of raw function pointer to type it in `std::function`. A lambda holds a type.

### Fragments

To group a list of children without adding extra nodes to the DOM or to use [DocumentFragments](https://developer.mozilla.org/en-US/docs/Web/API/DocumentFragment) to improve the performance of the app, the node can be created with an empty selector.

```cpp
// without fragments to return 3 div
// A parent node must be added to be inserted into the DOM tree
/* wasmdom::VNode vnode =
    div()({
        div()(std::string("Child 1")),
        div()(std::string("Child 2")),
        div()(std::string("Child 3"))
      }
    );
*/

// with fragments, just add them without additional nodes
wasmdom::VNode vnode =
  fragment()({ // or VNode("")
      div()(std::string("Child 1")),
      div()(std::string("Child 2")),
      div()(std::string("Child 3"))
    }
  );
```

### Server Side Rendering

To do server-side rendering, wasm-dom provides 2 simple steps:

1. Use `toHTML` to generate HTML on the server and send it to the client for faster page loads and to allow search engines to crawl your pages for SEO purposes.
2. After that, call `toVNode` on the node previously server-rendered and patch it with a vnode created on the client. In this way wasm-dom will preserve it and only attach event handlers, providing a fantastic first-load experience.

```cpp
// a function that returns the view, used on client and server
VNode view() {
  return
    div(("id", "root"s))({
      h1()(std::string("Title")),
      button(("class", "btn"s), ("onclick", f(onButtonClick)))(
        "Click Me!"
      )
    });
}

// on the server
VNode vnode = view();
std::string appString = vnode.toHTML();
std::string html =
  "<!DOCTYPE html>"
  "<html>"
    "<head>"
      "<title>My Awesome App</title>"
      "<link rel=\"stylesheet\" href=\"/index.css\" />"
    "</head>"

    "<body>"
      + appString +
    "</body>"

    "<script src=\"/bundle.js\"></script>"
  "</html>";

// on the client
VNode oldVNode = VNode::toVNode(
  emscripten::val::global("document").call<emscripten::val>("getElementById", emscripten::val("root"))
);
VNode vnode = view();

VDom vdom(oldVNode);
vdom.patch(vnode); // attach event handlers
```

### Web Components

Virtual DOM and WebComponents represent different technologies. Virtual DOM provides a declarative way to write the UI and keep it in sync with the data, while WebComponents provide encapsulation for reusable components. There are no limitations to using them together, use wasm-dom with WebComponents or use wasm-dom inside WebComponents.

#### Using WebComponents in wasm-dom

With wasm-dom you can just use WebComponents as any other element.

```cpp
// customElements.define('my-tabs', MyTabs);

VNode vnode =
  VNode("my-tabs",
        ("class", "css-class"s),
        ("attr", "an attribute"s),
        ("prop", emscripten::val("a prop")),
        ("tab-select", f(onTabSelect)))({
    p()(std::string("I'm a child!"))
  });
```

#### Using wasm-dom in WebComponents

At the moment creating WebComponents from C++ is not so easy, mixing some C++ and Javascript code is probably needed. Maybe with [emscripten_run_script](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#calling-javascript-from-c-c), [EM_ASM](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#calling-javascript-from-c-c) or [Embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html).

### API

#### Init

The `init` function has to be called before using wasm-dom, in the main function, to prepare its environment.

#### Nodes

Attributes can contain 2 special keys:

- ns: the namespace URI to associate with the element
- key: this property is used to keep pointers to DOM nodes that existed previously to avoid recreating them if it is unnecessary. This is very useful for things like list reordering.

```cpp
wasmdom::VNode vnode =
  wasmdom::dsl::div(("style", "color: #000"s))({
    wasmdom::dsl::h1()("Headline"),
    wasmdom::dsl::p()("A paragraph")
  });

wasmdom::VNode vnode2 =
  wasmdom::dsl::div(("id", "an-id"s), // node.setAttribute('id', 'an-id')
                    ("key", "foo"s),
                    ("class", "foo"s), // node.setAttribute('class', 'foo')
                    ("data-foo", "bar"s), // a dataset attribute
                    ("foo", emscripten::val(7)), // node.foo = 7
                    // function pointer
                    ("ondblclick", f(onDblClick)),
                    // lambda
                    ("onclick", [](emscripten::val e) -> bool {
                      // do stuff...
                      return true;
                    })
  );
```

##### Patch

Create a VDom object with a DOM element (using [emscripten::val::global](https://emscripten.org/docs/api_reference/val.h.html) for example) or a vnode representing the current view.

Then call patch with a vnode representing the new, updated view. If patch succeeded, the new vnode is returned.

If a DOM element is passed, newVnode will be turned into a DOM node, and the passed element will be replaced by the created DOM node. If an oldVnode is passed, wasm-dom will efficiently modify it to match the description in the new vnode.

```cpp
VNode oldVnode = span()(std::string("old node"));
VNode newVnode = span()(std::string("new node"));

VDom vdom(
    emscripten::val::global("document").call<emscripten::val>("getElementById", emscripten::val("root")),
);
vdom.patch(oldVnode);
vdom.patch(newVnode);

vdom.patch(newVnode); // do nothing, return newVnode
```

## License

Copyright (c) 2016 Matteo Basso as part of [asm-dom](https://github.com/mbasso/asm-dom)<br/>
Copyright (c) 2025 Théo Devaucoup

[Back to top](#top)
