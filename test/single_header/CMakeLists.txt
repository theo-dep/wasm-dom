add_executable(single_header_test
    ../jsdom.cpp
    test1.cpp
    test2.cpp
)

target_include_directories(single_header_test PRIVATE .. ../../extra)
target_link_options(single_header_test PRIVATE -lembind)
target_link_libraries(single_header_test PRIVATE Catch2::Catch2WithMain)

add_dependencies(single_header_test single_header)

add_test(NAME singleHeader
    COMMAND ${NODE_JS_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/single_header_test.js --colour-mode none
)
