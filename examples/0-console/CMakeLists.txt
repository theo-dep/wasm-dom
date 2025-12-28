add_executable(console main.cpp)

target_link_libraries(console PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags examples_options)

set(EXAMPLE_TARGET console.js PARENT_SCOPE)
set(EXAMPLE_TITLE Console PARENT_SCOPE)
