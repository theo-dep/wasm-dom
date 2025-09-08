add_executable(events main.cpp)

target_link_libraries(events PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(events PRIVATE -sENVIRONMENT=web)

include(../configure_html_file.cmake)
configure_html_file(events.js Events)
