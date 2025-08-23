add_executable(web-benchmarks main.cpp)

target_link_libraries(web-benchmarks PRIVATE wasm-dom compile_flags disable_exceptions optimization_flags)
target_link_options(web-benchmarks PRIVATE -sENVIRONMENT=web -sALLOW_MEMORY_GROWTH)

include(../configure_html_file.cmake)
configure_html_file(web-benchmarks.js Benchmarks)
