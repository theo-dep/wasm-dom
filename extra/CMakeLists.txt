file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.hpp")
file(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.cpp")
list(PREPEND HEADER_FILES
    ${erased_SOURCE_DIR}/erased/include/erased/utils/utils.h
    ${erased_SOURCE_DIR}/erased/include/erased/erased.h
)

set(OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/wasm-dom.hpp)

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)
if(CLANG_FORMAT_EXECUTABLE)
    set(CLANG_FORMAT_COMMAND COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${OUTPUT_FILE})
endif()

add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_NAME="${PROJECT_NAME}"
        -DHEADER_FILES="${HEADER_FILES}"
        -DSOURCE_FILES="${SOURCE_FILES}"
        -DOUTPUT_FILE="${OUTPUT_FILE}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake --log-level=debug
    ${CLANG_FORMAT_COMMAND}
    DEPENDS ${HEADER_FILES} ${SOURCE_FILES}
    COMMENT "Generating single header file ${OUTPUT_FILE}"
)

add_custom_target(single_header ALL DEPENDS ${OUTPUT_FILE})

set(TEST_MAIN ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)
file(WRITE ${TEST_MAIN} "#include <${OUTPUT_FILE}>\n\nint main() {\n wasmdom::VNode vnode(nullptr);\n return 0;\n}\n")

add_executable(single_header_test ${TEST_MAIN})
target_link_options(single_header_test PRIVATE -lembind)
add_dependencies(single_header_test single_header)
