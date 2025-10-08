cmake_minimum_required(VERSION 3.20...3.25)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# detect if wasm-dom is being bundled,
# keep only library in that case
if(NOT DEFINED PROJECT_NAME)
    set(WASM_DOM_NOT_SUBPROJECT ON)
else()
    set(WASM_DOM_NOT_SUBPROJECT OFF)
endif()

option(WASM_DOM_TESTS "Build and add tests to test runner" ${WASM_DOM_NOT_SUBPROJECT})
option(WASM_DOM_EXAMPLES "Build examples" ${WASM_DOM_NOT_SUBPROJECT})
option(WASM_DOM_ASAN "Enable Address Sanitizer flags in debug" ${WASM_DOM_NOT_SUBPROJECT})
option(WASM_DOM_SINGLE_HEADER "Construct the single header" ${WASM_DOM_NOT_SUBPROJECT})
option(WASM_DOM_STRICT "Enable strict compilation" ${WASM_DOM_NOT_SUBPROJECT})
option(WASM_DOM_NO_EXCEPTION "Disable exceptions" ${WASM_DOM_NOT_SUBPROJECT})
option(WASM_DOM_FULL_OPTIMISATION "Disable RTTI and filesystem, enable clojure compiler" ${WASM_DOM_NOT_SUBPROJECT})

set(WASM_DOM_COVERAGE OFF CACHE BOOL "Enable code coverage instrumentation")
set(WASM_DOM_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

project(wasm-dom LANGUAGES C CXX VERSION 0.2.0)

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
