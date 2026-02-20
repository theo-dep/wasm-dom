set(TARGET_NAME wasm-dom)

if(EMSCRIPTEN)
    add_library(wasm-dom-internals-jsapi OBJECT internals/jsapi.c)
endif()

add_library(wasm-dom STATIC
    attribute.cpp
    vnode.cpp
)

if(EMSCRIPTEN)
    target_sources(wasm-dom PRIVATE
        internals/domapi.cpp
        internals/domrecycler.cpp
        internals/wire.cpp
        vdom.cpp
    )
endif()

target_include_directories(wasm-dom PUBLIC ${WASM_DOM_SOURCE_DIR}/include)
if(EMSCRIPTEN)
    target_link_libraries(wasm-dom PUBLIC $<TARGET_OBJECTS:wasm-dom-internals-jsapi>)
    target_link_options(wasm-dom PUBLIC -lembind)
endif()

target_include_directories(wasm-dom PRIVATE .)
target_link_libraries(wasm-dom PRIVATE address_sanitizer_compile compile_flags optimization_flags disable_exceptions)

if(WASM_DOM_COVERAGE)
    target_compile_definitions(wasm-dom PUBLIC WASMDOM_COVERAGE)
    target_link_libraries(wasm-dom PUBLIC llvm-gcov)
endif()
