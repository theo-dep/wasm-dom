add_executable(web-component main.cpp)

target_link_libraries(web-component PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(web-component PRIVATE -sENVIRONMENT=web)

include(../configure_html_file.cmake)
configure_html_file(web-component.js WebComponent)
