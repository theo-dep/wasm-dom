add_executable(web-component main.cpp)

target_link_libraries(web-component PRIVATE wasm-dom)

include(../configure_html_file.cmake)
configure_html_file(web-component.js WebComponent)
