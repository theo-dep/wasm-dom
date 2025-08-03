add_executable(counter main.cpp)

target_link_libraries(counter PRIVATE wasm-dom)

include(../configure_html_file.cmake)
configure_html_file(counter.js Counter)
