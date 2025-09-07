set(TARGET_NAME wasm-dom)

add_library(wasm-dom-internals-jsapi OBJECT wasm-dom/internals/jsapi.c)

add_library(wasm-dom
    wasm-dom/attribute.cpp
    wasm-dom/domapi.cpp
    wasm-dom/domrecycler.cpp
    wasm-dom/vdom.cpp
    wasm-dom/vnode.cpp
)

target_include_directories(wasm-dom PUBLIC .)
target_link_libraries(wasm-dom PUBLIC $<TARGET_OBJECTS:wasm-dom-internals-jsapi>)
target_link_libraries(wasm-dom PRIVATE compile_flags optimization_flags disable_exceptions)

if(WASM_DOM_COVERAGE)
    target_compile_definitions(wasm-dom PUBLIC WASMDOM_COVERAGE)
    target_link_libraries(wasm-dom PUBLIC llvm-gcov)
endif()
