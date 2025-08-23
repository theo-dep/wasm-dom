file(GLOB_RECURSE HEADER_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.hpp")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.cpp")

# clean jsapi.c content
file(READ ${CMAKE_SOURCE_DIR}/src/wasm-dom/internals/jsapi.c JSAPI_CONTENT)
string(REGEX REPLACE "#include <.*>\n\n" "" JSAPI_CONTENT "${JSAPI_CONTENT}")
string(REGEX REPLACE "typedef struct _EM_VAL\\* EM_VAL;\n" "" JSAPI_CONTENT "${JSAPI_CONTENT}")
string(REGEX REPLACE "EM_VAL" "emscripten::EM_VAL" JSAPI_CONTENT "${JSAPI_CONTENT}")

# add jsapi.c content in jsapi.hpp
set(JSAPI_FILE ${CMAKE_BINARY_DIR}/src/wasm-dom/internals/jsapi.hpp)
file(WRITE ${JSAPI_FILE} "#pragma once\n\n#include <emscripten/em_js.h>\n#include <emscripten/val.h>\n\nnamespace wasmdom::internals::jsapi {\n\n${JSAPI_CONTENT}\n}")

# replace jsapi.hpp by the generated one
list(FIND HEADER_FILES ${CMAKE_SOURCE_DIR}/src/wasm-dom/internals/jsapi.hpp JSAPI_INDEX)
list(REMOVE_AT HEADER_FILES ${JSAPI_INDEX})
list(INSERT HEADER_FILES ${JSAPI_INDEX} ${JSAPI_FILE})

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
        -DOUTPUT_FILE="${OUTPUT_FILE}"
        --log-level=${CMAKE_LOG_LEVEL}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake
    ${CLANG_FORMAT_COMMAND}
    DEPENDS ${HEADER_FILES} ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake
    COMMENT "Generating single header file ${OUTPUT_FILE}"
)

add_custom_target(single_header ALL DEPENDS ${OUTPUT_FILE})

set(TEST_MAIN ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)
file(WRITE ${TEST_MAIN}
        "#include <${OUTPUT_FILE}>\n"
        "int main() {\n"
        "     wasmdom::VNode vnode = wasmdom::dsl::div();\n"
        "     wasmdom::VDom vdom(emscripten::val(\"\"));\n"
        "     vdom.patch(vnode);\n"
        "     return 0;\n"
        "}\n"
)

add_executable(single_header_test ${TEST_MAIN})
target_link_options(single_header_test PRIVATE -lembind)
add_dependencies(single_header_test single_header)
