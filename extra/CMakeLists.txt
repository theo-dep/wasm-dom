file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.cpp")

set(JSAPI_C_FILE ${CMAKE_SOURCE_DIR}/src/wasm-dom/internals/jsapi.c)

set(OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/wasm-dom.hpp)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    set(CLANG_FORMAT_COMMAND COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${OUTPUT_FILE})
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.25)
    cmake_language(GET_MESSAGE_LOG_LEVEL CMAKE_LOG_LEVEL)
else()
    set(CMAKE_LOG_LEVEL "STATUS")
endif()

add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_NAME="${PROJECT_NAME}"
        -DHEADER_FILES="${HEADER_FILES}"
        -DSOURCE_FILES="${SOURCE_FILES}"
        -DJSAPI_C_FILE="${JSAPI_C_FILE}"
        -DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
        -DBINARY_DIR="${CMAKE_BINARY_DIR}"
        -DOUTPUT_FILE="${OUTPUT_FILE}"
        --log-level=${CMAKE_LOG_LEVEL}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake
    ${CLANG_FORMAT_COMMAND}
    DEPENDS ${HEADER_FILES} ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake ${JSAPI_C_FILE}
    COMMENT "Generating single header file ${OUTPUT_FILE}"
)

add_custom_target(single_header ALL DEPENDS ${OUTPUT_FILE})

set(TEST_TEST_CPP ${CMAKE_CURRENT_BINARY_DIR}/test.cpp)
file(WRITE ${TEST_TEST_CPP}
        "#include <${OUTPUT_FILE}>\n"
        "wasmdom::VNode test() {\n"
        "     return wasmdom::dsl::div();\n"
        "}\n"
)

set(TEST_TEST_HPP ${CMAKE_CURRENT_BINARY_DIR}/test.hpp)
file(WRITE ${TEST_TEST_HPP}
        "namespace wasmdom { class VNode; }\n"
        "wasmdom::VNode test();\n"
)

set(TEST_MAIN ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)
file(WRITE ${TEST_MAIN}
        "#include \"${TEST_TEST_HPP}\"\n"
        "#include <${OUTPUT_FILE}>\n"
        "int main() {\n"
        "     wasmdom::VNode vnode = test();\n"
        "     wasmdom::VDom vdom(emscripten::val(\"\"));\n"
        "     vdom.patch(vnode);\n"
        "     return 0;\n"
        "}\n"
)

add_executable(single_header_test ${TEST_MAIN} ${TEST_TEST_CPP})
target_link_options(single_header_test PRIVATE -lembind)
add_dependencies(single_header_test single_header)
