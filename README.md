# wasm-dom

[![experimental](http://badges.github.io/stability-badges/dist/experimental.svg)](http://github.com/badges/stability-badges)
[![Build with Emscripten](https://github.com/theo-dep/wasm-dom/actions/workflows/pr.yml/badge.svg)](https://github.com/theo-dep/wasm-dom/actions/workflows/pr.yml)
[![codecov](https://codecov.io/github/theo-dep/wasm-dom/graph/badge.svg?token=Q96WRJMWUJ)](https://codecov.io/github/theo-dep/wasm-dom)

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
- Create VDom class and allocate VNode on stack to prepare for a domain-specific language (DSL).
  Less than 50% slower relative to initial code before [PR #3](https://github.com/theo-dep/wasm-dom/pull/3) for patch with addition. Others performances are stable.
- Add a domain-specific language (DSL) with attributes in key-value pairs from namespace `wasmdom::dsl`. Text and children are now added with `operator()`. Remove `h` function.

## Motivation (from asm-dom author)

> asm-dom is a minimal WebAssembly virtual DOM to build C++ SPA (Single page applications). You can write an entire SPA in C++ and compile it to WebAssembly (or asmjs as fallback) using [Emscripten](https://emscripten.org/), asm-dom will call DOM APIs for you. This will produce an app that `aims to execute at native speed by taking advantage of common hardware capabilities`, also, you can use your C/C++ code without any change, you haven't to create a binding layer to use it (as we have to do if we want to use a C++ lib from JS). Basically we are creating an app in C++ that call javascript if needed instead of the opposite. You can write only once in C++ and share as much code as possible with desktop/mobile apps and web site. If you want to learn more about performance, please see [this](https://github.com/mbasso/asm-dom/tree/master/benchmarks).
>
> _How can I structure my application with asm-dom?_
>
> asm-dom is a low-level virtual DOM library. It is unopinionated with regards to how you should structure your application.
>
> _How did you come up with the concept of asm-dom?_
>
> At the beginning asm-dom is born from the idea to test the powerful of WebAssembly in a common use case that is not gaming, VR, AR or Image / video editing. Unfortunately, at the moment, [GC/DOM Integration](http://webassembly.org/docs/future-features/) is a future feature, so, asm-dom isn't totally developed in wasm. All interactions with the DOM are written in Javascript. This is a big disadvantage because of the overhead of the binding between JS and WASM, in the future asm-dom will be even more powerful, anyway results are satisfying.

## Inline Example

```c++
#include "wasm-dom.hpp"

using namespace wasmdom;
using namespace wasmdom::dsl;

int main() {
  init();

  VNode vnode =
    div(("onclick", [](emscripten::val e) -> bool {
          emscripten::val::global("console").call<void>("log", emscripten::val("clicked"));
          return true;
        })
    )({
        span(("style", "font-weight: bold"s))(
          "This is bold"
        )
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

## Example

- [Console example](/examples/console/main.cpp)

## Getting started

wasm-dom aims to be used from C++, here you can find the doc:

- [C++ docs](/docs/installation.md)
