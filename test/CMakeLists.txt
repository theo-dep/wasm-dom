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

set(LINK_OPTIONS -sENVIRONMENT=node -sMODULARIZE -sEXPORT_NAME=Module -sEXIT_RUNTIME -sNO_DISABLE_EXCEPTION_CATCHING
    $<$<CONFIG:Debug>:-gsource-map -fsanitize=address -sALLOW_MEMORY_GROWTH>
)

add_executable(tests utils.cpp)

target_include_directories(tests PRIVATE .)
target_link_libraries(tests PRIVATE wasm-dom Catch2::Catch2WithMain)
target_link_options(tests PRIVATE ${LINK_OPTIONS})

set(TEST_CASES attributes dataset domRecycler eventListeners h patch props toHTML toVNode)

foreach(test ${TEST_CASES})
    string(TOLOWER ${test} test_src)
    target_sources(tests PRIVATE src/${test_src}.cpp)
    add_test(NAME ${test}
        COMMAND ${NODE_JS_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/runner.js" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/tests.js" "[${test}]" --colour-mode none
    )
endforeach()

add_executable(benchmarks utils.cpp benchmarks.cpp)

target_link_libraries(benchmarks PRIVATE wasm-dom Catch2::Catch2WithMain)
target_link_options(benchmarks PRIVATE ${LINK_OPTIONS} -sALLOW_MEMORY_GROWTH)

set(CMAKE_CTEST_ARGUMENTS "--verbose")
add_test(NAME benchmarks
    COMMAND ${NODE_JS_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/runner.js" "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/benchmarks.js" --colour-mode none
)
