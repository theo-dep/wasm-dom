set(ERASED_BUILD_TESTS OFF)
set(ERASED_BUILD_BENCHMARKS OFF)
FetchContent_Declare(
    Erased
    GIT_REPOSITORY https://github.com/theo-dep/Erased
    GIT_TAG        0abb91108378862c65899a452a149fd514ec0f21
)
FetchContent_MakeAvailable(Erased)

set(TARGET_NAME wasm-dom)

add_library(${TARGET_NAME}
    wasm-dom/domapi.cpp
    wasm-dom/domrecycler.cpp
    wasm-dom/vdom.cpp
    wasm-dom/vnode.cpp
)

target_include_directories(${TARGET_NAME} PUBLIC .)
target_link_libraries(${TARGET_NAME} PUBLIC erased::erased embind)

if(WASM_DOM_COVERAGE)
    target_compile_definitions(${TARGET_NAME} PUBLIC WASMDOM_COVERAGE)
    target_link_libraries(${TARGET_NAME} PUBLIC llvm-gcov)
endif()
