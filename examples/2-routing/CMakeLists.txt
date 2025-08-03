add_executable(routing main.cpp)

target_link_libraries(routing PRIVATE wasm-dom)

include(../configure_html_file.cmake)
configure_html_file(routing.js Routing)
