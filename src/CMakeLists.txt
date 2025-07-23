set(TARGET_NAME wasm-dom)

add_library(${TARGET_NAME}
    wasm-dom/config.cpp
    wasm-dom/diff.cpp
    wasm-dom/h.cpp
    wasm-dom/init.cpp
    wasm-dom/patch.cpp
    wasm-dom/tohtml.cpp
    wasm-dom/tovnode.cpp
    wasm-dom/vnode.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC .)

target_link_options(${TARGET_NAME} PUBLIC -lembind -sEXPORTED_RUNTIME_METHODS=['UTF8ToString'])

if(WASM_DOM_COVERAGE)
    target_link_libraries(${TARGET_NAME} PUBLIC ncnn-gcov)
endif()
