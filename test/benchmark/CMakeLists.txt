add_executable(benchmarks
    ../jsdom.cpp
    benchmarks.cpp
)

target_include_directories(benchmarks PRIVATE ..)
target_link_libraries(benchmarks PRIVATE wasm-dom address_sanitizer_link compile_flags optimization_flags Catch2::Catch2WithMain)
target_link_options(benchmarks PRIVATE -sALLOW_MEMORY_GROWTH)

set(CMAKE_CTEST_ARGUMENTS --verbose)
add_test(NAME benchmarks
    COMMAND ${NODE_JS_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/benchmarks.js --colour-mode none
)
