cmake_minimum_required (VERSION 3.26)
project(wasm-dom LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WASM_DOM_TESTS "Build and add tests to test runner" ON)
option(WASM_DOM_EXAMPLES "Build examples" ON)

add_subdirectory(src)

if(WASM_DOM_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()

if(WASM_DOM_EXAMPLES)
    add_subdirectory(examples/console)
endif()
