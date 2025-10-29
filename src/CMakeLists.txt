set(TARGET_NAME wasm-dom)

add_library(wasm-dom-internals-jsapi OBJECT internals/jsapi.c)

add_library(wasm-dom STATIC
    internals/domapi.cpp
    internals/domrecycler.cpp
    internals/wire.cpp
    attribute.cpp
    vdom.cpp
    vnode.cpp
)

target_include_directories(wasm-dom PUBLIC ${WASM_DOM_SOURCE_DIR}/include)
target_link_libraries(wasm-dom PUBLIC $<TARGET_OBJECTS:wasm-dom-internals-jsapi>)
target_link_options(wasm-dom PUBLIC -lembind)

target_include_directories(wasm-dom PRIVATE .)
target_link_libraries(wasm-dom PRIVATE address_sanitizer_compile compile_flags optimization_flags disable_exceptions)

if(WASM_DOM_COVERAGE)
    target_compile_definitions(wasm-dom PUBLIC WASMDOM_COVERAGE)
    target_link_libraries(wasm-dom PUBLIC llvm-gcov)
endif()
