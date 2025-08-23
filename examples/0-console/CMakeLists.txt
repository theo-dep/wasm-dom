add_executable(console main.cpp)

target_link_libraries(console PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(console PRIVATE -sENVIRONMENT=web)

include(../configure_html_file.cmake)
configure_html_file(console.js Console)
