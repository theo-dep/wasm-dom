# Change Log

This project adheres to [Semantic Versioning](http://semver.org/).
Every release, along with the migration instructions, is documented on the Github [Releases](https://github.com/theo-dep/wasm-dom/releases) page.

## v0.1.0

- Remove JavaScript side, this version kept only the C++ compatibility.
- Create VDom class and allocate VNode on stack to prepare for a domain-specific language (DSL).
- Add a domain-specific language (DSL) with attributes in key-value pairs from namespace `wasmdom::dsl`. Text and children are now added with `operator()`. Remove `h` function.
- A single header in [extra](/extra) folder.

## v0.2.0

- Remove the `init` method and create DOM API functions and DomRecycler singleton class.
- Add [WebAssembly Garbage Collector](https://github.com/WebAssembly/gc) support but keep DOM recycler for old browser ([very recent feature](https://webassembly.org/features/#table-row-gc)).
- Improve performance by 50%. See [PR #14](https://github.com/theo-dep/wasm-dom/pull/14) to follow.
- Can be used without exceptions and RTTI to [optimize code size](https://emscripten.org/docs/optimizing/Optimizing-Code.html#c-rtti).
- Replace `"ref"` callbacks by `onMount`, `onUpdate` and `onUnmount` event callbacks.

## v0.2.1

- Enable wasm-dom integration as either a Git submodule or via CMake’s [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html).
- Introduce build options to compile only the library.

## v0.3.0

- Update `patch` method to support many fragment node configurations.
- Allow `fragment` with attribute for `key` and event callbacks.
- Build `VNode::toHTML` for native platform (Linux, macOS and Windows).
