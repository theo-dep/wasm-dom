add_executable(console main.cpp)

target_link_libraries(console PRIVATE wasm-dom)

include(../configure_html_file.cmake)
configure_html_file(console.js Console)
