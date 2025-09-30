add_executable(counter main.cpp)

target_link_libraries(counter PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(counter PRIVATE -sENVIRONMENT=web)

set(EXAMPLE_TARGET counter.js PARENT_SCOPE)
set(EXAMPLE_TITLE Counter PARENT_SCOPE)
