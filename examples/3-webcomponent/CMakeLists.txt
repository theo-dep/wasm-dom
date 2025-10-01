add_executable(web-component main.cpp)

target_link_libraries(web-component PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(web-component PRIVATE -sENVIRONMENT=web)

set(EXAMPLE_TARGET web-component.js PARENT_SCOPE)
set(EXAMPLE_TITLE WebComponent PARENT_SCOPE)
