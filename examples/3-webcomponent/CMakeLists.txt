add_executable(web-component main.cpp)

target_link_libraries(web-component PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags examples_options)
target_compile_options(web-component PRIVATE -Wno-dollar-in-identifier-extension)

set(EXAMPLE_TARGET web-component.js PARENT_SCOPE)
set(EXAMPLE_TITLE WebComponent PARENT_SCOPE)
