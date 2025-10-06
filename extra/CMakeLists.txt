file(GLOB_RECURSE HEADER_FILES
    "${WASM_DOM_SOURCE_DIR}/src/wasm-dom/*.h"
    "${WASM_DOM_SOURCE_DIR}/src/wasm-dom/*.hpp"
)
file(GLOB_RECURSE SOURCE_FILES "${WASM_DOM_SOURCE_DIR}/src/wasm-dom/*.cpp")

set(JSAPI_C_FILE ${WASM_DOM_SOURCE_DIR}/src/wasm-dom/internals/jsapi.c)

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
        -DSOURCE_DIR="${WASM_DOM_SOURCE_DIR}"
        -DBINARY_DIR="${CMAKE_BINARY_DIR}"
        -DOUTPUT_FILE="${OUTPUT_FILE}"
        --log-level=${CMAKE_LOG_LEVEL}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake
    ${CLANG_FORMAT_COMMAND}
    DEPENDS ${HEADER_FILES} ${SOURCE_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake ${JSAPI_C_FILE}
    COMMENT "Generating single header file ${OUTPUT_FILE}"
)

add_custom_target(single_header ALL DEPENDS ${OUTPUT_FILE})
