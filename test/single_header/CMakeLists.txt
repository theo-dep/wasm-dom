add_executable(single_header_test
    tohtml.cpp
)

if(EMSCRIPTEN)
    target_sources(single_header_test PRIVATE
        test1.cpp
        test2.cpp
    )
endif()

target_include_directories(single_header_test PRIVATE ${WASM_DOM_SOURCE_DIR}/extra)
target_link_libraries(single_header_test PRIVATE compile_flags Catch2::Catch2WithMain)

add_dependencies(single_header_test single_header)

set(TEST_EXTENSION)

if(EMSCRIPTEN)
    target_link_options(single_header_test PRIVATE -lembind)
    target_link_libraries(single_header_test PRIVATE common)

    set(TEST_EXTENSION .js)
endif()

add_test(NAME singleHeader
    COMMAND ${CMAKE_CROSSCOMPILING_EMULATOR} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/single_header_test${TEST_EXTENSION}
        --colour-mode none --reporter compact
)
