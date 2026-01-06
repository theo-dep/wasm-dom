add_executable(counter main.cpp)

target_link_libraries(counter PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags examples_options)

set(EXAMPLE_TARGET counter.js PARENT_SCOPE)
set(EXAMPLE_TITLE Counter PARENT_SCOPE)
