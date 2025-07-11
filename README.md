# wasm-dom

[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)

> A minimal WebAssembly virtual DOM to build C++ SPA (Single page applications)

## Table of Contents

- [History](#history)
- [Motivation](#motivation)
- [Inline Example](#inline-example)
- [Examples](#examples)
- [Getting started](#getting-started)

## History

Initial version of wasm-dom is fork of [asm-dom](https://github.com/mbasso/asm-dom) version 0.7.0, and there is no intention to keep it compatible with it.

### Changelog (since fork)

- Remove JavaScript side, this version kept only the c++ compatibility.

## Motivation

wasm-dom is a minimal WebAssembly virtual DOM to build C++ SPA (Single page applications). You can write an entire SPA in C++ and compile it to WebAssembly (or asmjs as fallback) using [Emscripten](https://emscripten.org/), wasm-dom will call DOM APIs for you. This will produce an app that `aims to execute at native speed by taking advantage of common hardware capabilities`, also, you can use your C/C++ code without any change, you haven't to create a binding layer to use it (as we have to do if we want to use a C++ lib from JS). Basically we are creating an app in C++ that call javascript if needed instead of the opposite. You can write only once in C++ and share as much code as possible with desktop/mobile apps and web site. If you want to learn more about performance, please see [this](https://github.com/mbasso/asm-dom/tree/master/benchmarks).

*How can I structure my application with wasm-dom?*

wasm-dom is a low-level virtual DOM library. It is unopinionated with regards to how you should structure your application.

*How did you come up with the concept of wasm-dom?*

At the beginning wasm-dom is born from the idea to test the powerful of WebAssembly in a common use case that is not gaming, VR, AR or Image / video editing. Unfortunately, at the moment, [GC/DOM Integration](http://webassembly.org/docs/future-features/) is a future feature 🦄, so, wasm-dom isn't totally developed in wasm. All interactions with the DOM are written in Javascript. This is a big disadvantage because of the overhead of the binding between JS and WASM, in the future wasm-dom will be even more powerful, anyway results are satisfying.

## Inline Example

```c++
#include "wasm-dom.hpp"

using namespace wasmdom;

int main() {
  Config config = Config();
  init(config);

  VNode* vnode = h("div",
    Data(
      Callbacks {
        {"onclick", [](emscripten::val e) -> bool {
          emscripten::val::global("console").call<void>("log", emscripten::val("clicked"));
          return true;
        }}
      }
    ),
    Children {
      h("span",
        Data(
          Attrs {
            {"style", "font-weight: bold"}
          }
        ),
        std::string("This is bold")
      ),
      h(" and this is just normal text", true),
      h("a",
        Data(
          Attrs {
            {"href", "/foo"}
          }
        ),
        std::string("I'll take you places!")
      )
    }
  );

  // Patch into empty DOM element – this modifies the DOM as a side effect
  patch(
    emscripten::val::global("document").call<emscripten::val>(
      "getElementById",
      std::string("root")
    ),
    vnode
  );

  return 0;
}
```

## Example

- [Console example](/examples/console/main.cpp)

## Getting started

wasm-dom aims to be used from C++, here you can find the doc:

- [C++ docs](/docs/installation.md)
