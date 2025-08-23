add_executable(counter main.cpp)

target_link_libraries(counter PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(counter PRIVATE -sENVIRONMENT=web)

include(../configure_html_file.cmake)
configure_html_file(counter.js Counter)
