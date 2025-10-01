add_executable(routing main.cpp)

target_link_libraries(routing PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(routing PRIVATE -sENVIRONMENT=web)

set(EXAMPLE_TARGET routing.js PARENT_SCOPE)
set(EXAMPLE_TITLE Routing PARENT_SCOPE)
