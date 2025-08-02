cmake_minimum_required(VERSION 3.22)
project(wasm-dom LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WASM_DOM_TESTS "Build and add tests to test runner" ON)
option(WASM_DOM_EXAMPLES "Build examples" ON)

set(WASM_DOM_COVERAGE OFF CACHE BOOL "Enable code coverage instrumentation")

add_compile_options(-Werror -Wall -Wextra -pedantic -sSTRICT -Wno-dollar-in-identifier-extension)

if(WASM_DOM_COVERAGE)
    add_subdirectory(coverage)
else()
    add_link_options(-sNO_FILESYSTEM)
endif()

add_subdirectory(src)
add_subdirectory(extra)

if(WASM_DOM_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()

if(WASM_DOM_EXAMPLES)
    add_subdirectory(examples)
endif()
