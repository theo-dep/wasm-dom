Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

find_program(NPM_EXECUTABLE npm)
if(NOT NPM_EXECUTABLE)
    message(FATAL "npm not found")
endif()

message(CHECK_START "Installing test dependencies")
execute_process(COMMAND ${NPM_EXECUTABLE} install WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
message(CHECK_PASS "done")

add_executable(tests main.cpp utils.cpp)

target_include_directories(tests PRIVATE .)
target_link_libraries(tests PRIVATE wasm-dom Catch2::Catch2)
target_link_options(tests PRIVATE -sENVIRONMENT=node -sMODULARIZE=1 -sEXPORT_NAME=Module -sEXIT_RUNTIME=1 -sNO_DISABLE_EXCEPTION_CATCHING)

set(TEST_CASES attributes dataset domRecycler eventListeners h index patch props toHTML toVNode)

foreach(test ${TEST_CASES})
    string(TOLOWER ${test} test_src)
    target_sources(tests PRIVATE src/${test_src}.cpp)
    add_test(NAME ${test}
        COMMAND ${NODE_JS_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/runner.js" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/tests.js" "[${test}]"
    )
endforeach()

add_executable(benchmarks main.cpp utils.cpp benchmarks.cpp)

target_link_libraries(benchmarks PRIVATE wasm-dom Catch2::Catch2)
target_link_options(benchmarks PRIVATE -sENVIRONMENT=node -sALLOW_MEMORY_GROWTH -sMODULARIZE=1 -sEXPORT_NAME=Module -sEXIT_RUNTIME=1 -sNO_DISABLE_EXCEPTION_CATCHING)

set(CMAKE_CTEST_ARGUMENTS "--verbose")
add_test(NAME benchmarks
    COMMAND ${NODE_JS_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/runner.js" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/benchmarks.js"
)
