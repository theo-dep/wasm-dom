add_executable(benchmarks
    benchmarks.cpp
)

target_include_directories(benchmarks PRIVATE ${WASM_DOM_SOURCE_DIR}/extra)
target_link_options(benchmarks PRIVATE -lembind -sALLOW_MEMORY_GROWTH)
target_link_libraries(benchmarks PRIVATE common compile_flags Catch2::Catch2WithMain)

set(CMAKE_CTEST_ARGUMENTS --verbose)
add_test(NAME benchmarks
    COMMAND ${NODE_JS_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/benchmarks.js
        --colour-mode none --reporter compact
)
