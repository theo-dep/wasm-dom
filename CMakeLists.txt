cmake_minimum_required(VERSION 3.21)
project(wasm-dom LANGUAGES C CXX VERSION 0.3.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(WASM_DOM_TESTS "Build and add tests to test runner" ${PROJECT_IS_TOP_LEVEL})
option(WASM_DOM_EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})
option(WASM_DOM_ASAN "Enable Address Sanitizer flags in debug" ${PROJECT_IS_TOP_LEVEL})
option(WASM_DOM_SINGLE_HEADER "Construct the single header" ${PROJECT_IS_TOP_LEVEL})
option(WASM_DOM_STRICT "Enable strict compilation" ${PROJECT_IS_TOP_LEVEL})
option(WASM_DOM_NO_EXCEPTION "Disable exceptions" ${PROJECT_IS_TOP_LEVEL})
option(WASM_DOM_FULL_OPTIMISATION "Disable RTTI and filesystem, enable clojure compiler" ${PROJECT_IS_TOP_LEVEL})

set(WASM_DOM_COVERAGE OFF CACHE BOOL "Enable code coverage instrumentation")
set(WASM_DOM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/address_sanitizer.cmake)
include(cmake/compile_flags.cmake)
include(cmake/disable_exceptions.cmake)
include(cmake/optimization_flags.cmake)

if(WASM_DOM_COVERAGE)
    add_subdirectory(coverage)
endif()

add_subdirectory(src)

if(WASM_DOM_SINGLE_HEADER)
    add_subdirectory(extra)
endif()

if(WASM_DOM_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()

if(WASM_DOM_EXAMPLES)
    add_subdirectory(examples)
endif()
