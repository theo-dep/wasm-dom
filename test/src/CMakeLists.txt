add_executable(tests)

target_include_directories(tests PRIVATE ${WASM_DOM_SOURCE_DIR}/src)
target_link_libraries(tests PRIVATE wasm-dom address_sanitizer_link compile_flags optimization_flags Catch2::Catch2WithMain)

set(TEST_CASES attributes h toHTML)
set(TEST_EXTENSION)

if(EMSCRIPTEN)
    # Stack overflow! Stack cookie has been overwritten in patch tests
    target_link_options(tests PRIVATE -sSTACK_SIZE=5MB)
    target_link_libraries(tests PRIVATE common)

    list(APPEND TEST_CASES dataset domApi domRecycler eventListeners patch patchEvent patchFragment props toVNode)
    set(TEST_EXTENSION .js)
endif()

foreach(test ${TEST_CASES})
    string(TOLOWER ${test} test_src)
    target_sources(tests PRIVATE ${test_src}.cpp)
    add_test(NAME src.${test}
        COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/tests${TEST_EXTENSION}
            [${test}] --colour-mode none --reporter compact
    )
endforeach()
