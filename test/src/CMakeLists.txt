add_executable(tests
    ../jsdom.cpp
)

target_include_directories(tests PRIVATE ${WASM_DOM_SOURCE_DIR}/test ${WASM_DOM_SOURCE_DIR}/src)
target_link_libraries(tests PRIVATE wasm-dom address_sanitizer_link compile_flags optimization_flags Catch2::Catch2WithMain)
target_link_options(tests PRIVATE ${LINK_OPTIONS})

set(TEST_CASES attributes dataset domApi domRecycler eventListeners h patch props toHTML toVNode)

foreach(test ${TEST_CASES})
    string(TOLOWER ${test} test_src)
    target_sources(tests PRIVATE ${test_src}.cpp)
    add_test(NAME ${test}
        COMMAND ${NODE_JS_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/tests.js [${test}] --colour-mode none
    )
endforeach()
