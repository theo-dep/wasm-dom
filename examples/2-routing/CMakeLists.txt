add_executable(routing main.cpp)

target_link_libraries(routing PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(routing PRIVATE -sENVIRONMENT=web)

include(../configure_html_file.cmake)
configure_html_file(routing.js Routing)
