file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.hpp")
file(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.cpp")

set(LAST_HPP_FILE "")
set(FIRST_HPP_FILES "")
foreach(FILE_PATH ${HEADER_FILES})
    get_filename_component(FILENAME ${FILE_PATH} NAME)
    if(FILENAME STREQUAL "h.hpp" OR FILENAME STREQUAL "vdom.hpp")
        set(LAST_HPP_FILE ${FILE_PATH})
    else()
        list(APPEND FIRST_HPP_FILES ${FILE_PATH})
    endif()
endforeach()

list(PREPEND SOURCE_FILES ${FIRST_HPP_FILES} ${LAST_HPP_FILE})

set(OUTPUT_FILE "${CMAKE_SOURCE_DIR}/extra/wasm-dom.hpp")

message(STATUS "Creating single header file")

file(WRITE ${OUTPUT_FILE} "")

file(APPEND ${OUTPUT_FILE} "// =============================================================================\n")
file(APPEND ${OUTPUT_FILE} "// Single Header Library\n")
file(APPEND ${OUTPUT_FILE} "// Auto-generated from multiple source files\n")
file(APPEND ${OUTPUT_FILE} "// Project: ${PROJECT_NAME}\n")
file(APPEND ${OUTPUT_FILE} "// =============================================================================\n")
file(APPEND ${OUTPUT_FILE} "#pragma once\n\n")

set(ALL_INCLUDES "")

foreach(FILE_PATH ${SOURCE_FILES})
    file(READ ${FILE_PATH} CONTENT)

    string(REGEX MATCHALL "#include[ \t]*<[^>]*>" SYSTEM_INCLUDES "${CONTENT}")
    foreach(INCLUDE ${SYSTEM_INCLUDES})
         string(APPEND ALL_INCLUDES "${INCLUDE}\n")
    endforeach()
endforeach()

string(REGEX REPLACE "\n+" "\n" ALL_INCLUDES "${ALL_INCLUDES}")
string(REGEX REPLACE "^(.+\n)" "\\1" UNIQUE_INCLUDES "${ALL_INCLUDES}")

string(REPLACE "\n" ";" INCLUDES_LIST "${ALL_INCLUDES}")
list(REMOVE_DUPLICATES INCLUDES_LIST)
list(SORT INCLUDES_LIST)

foreach(INCLUDE ${INCLUDES_LIST})
    file(APPEND ${OUTPUT_FILE} "${INCLUDE}\n")
endforeach()

file(APPEND ${OUTPUT_FILE} "\n")

list(LENGTH SOURCE_FILES FILE_COUNT)
message(STATUS "Found ${FILE_COUNT} files to include:")

foreach(FILE_PATH ${SOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${FILE_PATH})
    message(STATUS "  Processing: ${REL_PATH}")

    file(READ ${FILE_PATH} CONTENT)

    string(REGEX REPLACE "#pragma once[^\n]*\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#include[^\n]*\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE\n.*WASMDOM_INLINE.*#else.*#endif" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifndef WASMDOM_COVERAGE(.*)#endif" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE.*#endif" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifndef WASMDOM_COVERAGE(.*)#else.*#endif" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE.*#else(.*)#endif" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "WASMDOM_INLINE" "inline" CONTENT "${CONTENT}")
    string(REGEX REPLACE "\n\n\n+" "\n\n" CONTENT "${CONTENT}")
    string(REGEX REPLACE "^[\n\r\t ]+" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "[\n\r\t ]+$" "" CONTENT "${CONTENT}")

    file(APPEND ${OUTPUT_FILE} "// -----------------------------------------------------------------------------\n")
    file(APPEND ${OUTPUT_FILE} "// ${REL_PATH}\n")
    file(APPEND ${OUTPUT_FILE} "// -----------------------------------------------------------------------------\n")
    file(APPEND ${OUTPUT_FILE} "${CONTENT}")
    file(APPEND ${OUTPUT_FILE} "\n\n")
endforeach()

file(APPEND ${OUTPUT_FILE} "// End of single header library\n")

find_program(CLANG_FORMAT_EXECUTABLE NAMES clang-format)

if(CLANG_FORMAT_EXECUTABLE)
    execute_process(
        COMMAND ${CLANG_FORMAT_EXECUTABLE} -i ${OUTPUT_FILE}
        RESULT_VARIABLE CLANG_FORMAT_RESULT
        ERROR_VARIABLE CLANG_FORMAT_ERROR
    )

    if(CLANG_FORMAT_RESULT EQUAL 0)
        message(STATUS "Single header formatted with clang-format")
    else()
        message(FATAL "clang-format failed: ${CLANG_FORMAT_ERROR}")
    endif()
else()
    message(STATUS "clang-format not found, skipping formatting")
endif()

message(STATUS "Single header created with ${FILE_COUNT} files: ${OUTPUT_FILE}")

set(TEST_MAIN ${CMAKE_CURRENT_BINARY_DIR}/main.cpp)
file(WRITE ${TEST_MAIN} "#include <${OUTPUT_FILE}>\n\nint main() {\n wasmdom::VNode vnode(nullptr); return 0;\n}\n")

try_compile(SINGLE_HEADER_COMPILES
    ${CMAKE_CURRENT_BINARY_DIR}/test
    ${TEST_MAIN}
    CMAKE_FLAGS -DCMAKE_CXX_STANDARD=23
    LINK_OPTIONS "-lembind" "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString']"
    OUTPUT_VARIABLE COMPILE_OUTPUT
)

if(SINGLE_HEADER_COMPILES)
    message(STATUS "Single header compilation: SUCCESS")
else()
    message(WARNING "Single header compilation: FAILED")
    message(FATAL "Compilation output:\n${COMPILE_OUTPUT}")
endif()
