add_executable(web-benchmarks main.cpp)

target_link_libraries(web-benchmarks PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags)
target_link_options(web-benchmarks PRIVATE ${LINK_OPTIONS} -sALLOW_MEMORY_GROWTH)

set(EXAMPLE_TARGET web-benchmarks.js PARENT_SCOPE)
set(EXAMPLE_TITLE Benchmarks PARENT_SCOPE)
