add_executable(console main.cpp)

target_link_libraries(console PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags)
target_link_options(console PRIVATE ${LINK_OPTIONS})

set(EXAMPLE_TARGET console.js PARENT_SCOPE)
set(EXAMPLE_TITLE Console PARENT_SCOPE)
