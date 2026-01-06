add_executable(routing main.cpp)

target_link_libraries(routing PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags examples_options)

set(EXAMPLE_TARGET routing.js PARENT_SCOPE)
set(EXAMPLE_TITLE Routing PARENT_SCOPE)
