separate_arguments(HEADER_FILES)
separate_arguments(SOURCE_FILES)

set(LAST_HPP_FILE "")
set(FIRST_HPP_FILES "")
foreach(FILE_PATH ${HEADER_FILES})
    get_filename_component(FILENAME ${FILE_PATH} NAME)
    if(FILENAME STREQUAL "h.hpp" OR FILENAME STREQUAL "vdom.hpp")
        list(APPEND LAST_HPP_FILE ${FILE_PATH})
    else()
        list(APPEND FIRST_HPP_FILES ${FILE_PATH})
    endif()
endforeach()

list(PREPEND SOURCE_FILES ${FIRST_HPP_FILES} ${LAST_HPP_FILE})

file(WRITE ${OUTPUT_FILE} "")

file(APPEND ${OUTPUT_FILE} "// =============================================================================\n")
file(APPEND ${OUTPUT_FILE} "// Single Header Library\n")
file(APPEND ${OUTPUT_FILE} "// Auto-generated from multiple source files\n")
file(APPEND ${OUTPUT_FILE} "// Project: ${PROJECT_NAME}\n")
file(APPEND ${OUTPUT_FILE} "// =============================================================================\n")
file(APPEND ${OUTPUT_FILE} "#pragma once\n\n")

set(SOURCE_BASENAMES "")
foreach(FILE_PATH ${SOURCE_FILES})
    get_filename_component(BASENAME ${FILE_PATH} NAME)
    list(APPEND SOURCE_BASENAMES ${BASENAME})
endforeach()

set(ALL_INCLUDES "")

foreach(FILE_PATH ${SOURCE_FILES})
    file(READ ${FILE_PATH} CONTENT)

    string(REGEX MATCHALL "#include[ \t]*[<\"][^<>\"]*[>\"]" ALL_INCLUDES_IN_FILE "${CONTENT}")
    foreach(INCLUDE ${ALL_INCLUDES_IN_FILE})
        string(REGEX REPLACE "#include[ \t]*[<\"]([^<>\"]*)[>\"]" "\\1" INCLUDE_FILE "${INCLUDE}")
        get_filename_component(INCLUDE_BASENAME ${INCLUDE_FILE} NAME)

        list(FIND SOURCE_BASENAMES ${INCLUDE_BASENAME} FOUND_INDEX)
        if(FOUND_INDEX EQUAL -1)
            string(APPEND ALL_INCLUDES "${INCLUDE}\n")
        endif()
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
message(DEBUG "Found ${FILE_COUNT} files to include:")

foreach(FILE_PATH ${SOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR}/../.. ${FILE_PATH})
    message(DEBUG "  Processing: ${REL_PATH}")

    file(READ ${FILE_PATH} CONTENT)

    string(REGEX REPLACE "#pragma once[^\n]*\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#include[^\n]*\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE\n.*WASMDOM_INLINE.*#else.*#endif" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifndef WASMDOM_COVERAGE(.[^#]*)#endif" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE.[^#]*#endif" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifndef WASMDOM_COVERAGE(.[^#]*)#else.[^#]*#endif" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE.[^#]*#else(.[^#]*)#endif" "\\1" CONTENT "${CONTENT}")
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
