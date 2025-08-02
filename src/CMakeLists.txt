set(TARGET_NAME wasm-dom)

add_library(${TARGET_NAME}
    wasm-dom/h.cpp
    wasm-dom/init.cpp
    wasm-dom/vdom.cpp
    wasm-dom/vnode.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC .)

target_link_options(${TARGET_NAME} PUBLIC -lembind -sEXPORTED_RUNTIME_METHODS=['UTF8ToString'])

if(WASM_DOM_COVERAGE)
    target_compile_definitions(${TARGET_NAME} PUBLIC WASMDOM_COVERAGE)
    target_link_libraries(${TARGET_NAME} PUBLIC llvm-gcov)
endif()
