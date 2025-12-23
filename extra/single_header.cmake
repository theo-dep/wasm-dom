# Script to generate single header file

# Convert space-separated strings back to CMake lists
separate_arguments(HEADER_FILES)
separate_arguments(SOURCE_FILES)

# clean jsapi.c content
file(READ ${JSAPI_C_FILE} JSAPI_CONTENT)
string(REGEX REPLACE "#include <.*>\n\n" "" JSAPI_CONTENT "${JSAPI_CONTENT}")
string(REGEX REPLACE "#include \".*\"\n\n" "" JSAPI_CONTENT "${JSAPI_CONTENT}")
string(REGEX REPLACE "typedef struct _EM_VAL\\* EM_VAL;\n" "" JSAPI_CONTENT "${JSAPI_CONTENT}")
string(REGEX REPLACE "EM_VAL" "emscripten::EM_VAL" JSAPI_CONTENT "${JSAPI_CONTENT}")

# add jsapi.c content in jsapi.hpp
set(JSAPI_FILE ${BINARY_DIR}/src/internals/jsapi.hpp)
file(WRITE ${JSAPI_FILE}
        "#pragma once\n"
        "#include <emscripten/em_js.h>\n"
        "#include <emscripten/val.h>\n"
        "namespace wasmdom::internals::jsapi {\n"
            "${JSAPI_CONTENT}\n"
        "}"
)

# replace jsapi.hpp by the generated one
list(FIND HEADER_FILES ${SOURCE_DIR}/src/internals/jsapi.hpp JSAPI_INDEX)
list(REMOVE_AT HEADER_FILES ${JSAPI_INDEX})
list(INSERT HEADER_FILES ${JSAPI_INDEX} ${JSAPI_FILE})

# Create filename to path mapping for all files
set(ALL_FILES ${HEADER_FILES} ${SOURCE_FILES})
foreach(FILE_PATH ${ALL_FILES})
    get_filename_component(FILENAME ${FILE_PATH} NAME)
    set(FILE_MAP_${FILENAME} ${FILE_PATH})
    list(APPEND ALL_FILENAMES ${FILENAME})
endforeach()

# Build dependency graph (only for headers)
foreach(FILE_PATH ${HEADER_FILES})
    get_filename_component(FILENAME ${FILE_PATH} NAME)
    file(READ ${FILE_PATH} CONTENT)

    # Find local includes (our project files)
    string(REGEX MATCHALL "#include[ \t]*\"([^\"]*)\"|#include[ \t]*<([^>]*)>" INCLUDES_IN_FILE "${CONTENT}")
    foreach(INCLUDE_MATCH ${INCLUDES_IN_FILE})
        # Extract filename from both quote and bracket includes
        if(INCLUDE_MATCH MATCHES "#include[ \t]*\"([^\"]*)\"")
            set(INCLUDE_FILE ${CMAKE_MATCH_1})
        elseif(INCLUDE_MATCH MATCHES "#include[ \t]*<([^>]*)>")
            set(INCLUDE_FILE ${CMAKE_MATCH_1})
        endif()

        get_filename_component(INCLUDE_BASENAME ${INCLUDE_FILE} NAME)

        # If this include is one of our header files, add dependency
        foreach(HEADER_PATH ${HEADER_FILES})
            get_filename_component(HEADER_NAME ${HEADER_PATH} NAME)
            if(INCLUDE_BASENAME MATCHES ".*\\.inl\\.hpp" AND HEADER_NAME STREQUAL INCLUDE_BASENAME)
                # Dependency inverse order for *.inl.hpp files
                list(APPEND DEPS_${INCLUDE_BASENAME} ${FILENAME})
                break()
            elseif(HEADER_NAME STREQUAL INCLUDE_BASENAME)
                list(APPEND DEPS_${FILENAME} ${INCLUDE_BASENAME})
                break()
            endif()
        endforeach()
    endforeach()
endforeach()

# Simple dependency resolution: headers first, ordered by dependencies
set(SORTED_HEADERS "")
set(PROCESSED "")

# Function to add header and its dependencies recursively
function(add_header_with_deps FILENAME)
    # Skip if already processed
    list(FIND PROCESSED ${FILENAME} ALREADY_PROCESSED)
    if(NOT ALREADY_PROCESSED EQUAL -1)
        return()
    endif()

    # Add dependencies first
    if(DEFINED DEPS_${FILENAME})
        foreach(DEP ${DEPS_${FILENAME}})
            add_header_with_deps(${DEP})
        endforeach()
    endif()

    # Add this header
    list(APPEND PROCESSED ${FILENAME})
    list(APPEND SORTED_HEADERS ${FILE_MAP_${FILENAME}})
    set(PROCESSED ${PROCESSED} PARENT_SCOPE)
    set(SORTED_HEADERS ${SORTED_HEADERS} PARENT_SCOPE)
endfunction()

# Process all headers with dependency order
foreach(FILE_PATH ${HEADER_FILES})
    get_filename_component(FILENAME ${FILE_PATH} NAME)
    add_header_with_deps(${FILENAME})
endforeach()

# Final order: sorted headers + all cpp files
set(SOURCE_FILES ${SORTED_HEADERS} ${SOURCE_FILES})

# Initialize output file
file(WRITE ${OUTPUT_FILE} "")

# File header
file(APPEND ${OUTPUT_FILE} "// =============================================================================\n")
file(APPEND ${OUTPUT_FILE} "// Single Header Library\n")
file(APPEND ${OUTPUT_FILE} "// Auto-generated from multiple source files\n")
file(APPEND ${OUTPUT_FILE} "// Project: ${PROJECT_NAME}\n")
file(APPEND ${OUTPUT_FILE} "// =============================================================================\n")
file(APPEND ${OUTPUT_FILE} "#pragma once\n\n")

# Collect all includes that are not local files
set(ALL_INCLUDES "")

foreach(FILE_PATH ${SOURCE_FILES})
    file(READ ${FILE_PATH} CONTENT)

    # Extract the filename from the include
    string(REGEX MATCHALL "#include[ \t]*<[^>]*>" SYSTEM_INCLUDES "${CONTENT}")
    # Only keep includes that are not in our source files
    foreach(INCLUDE ${SYSTEM_INCLUDES})
        if(NOT INCLUDE MATCHES "wasm-dom/.*")
            string(APPEND ALL_INCLUDES "${INCLUDE}\n")
        endif()
    endforeach()
endforeach()

# Add unique includes
string(REGEX REPLACE "\n+" "\n" ALL_INCLUDES "${ALL_INCLUDES}")
string(REGEX REPLACE "^(.+\n)" "\\1" UNIQUE_INCLUDES "${ALL_INCLUDES}")

string(REPLACE "\n" ";" ALL_INCLUDES "${ALL_INCLUDES}")
list(REMOVE_DUPLICATES ALL_INCLUDES)
list(SORT ALL_INCLUDES)

foreach(INCLUDE ${ALL_INCLUDES})
    file(APPEND ${OUTPUT_FILE} "${INCLUDE}\n")
endforeach()

file(APPEND ${OUTPUT_FILE} "\n")

# Process each file
list(LENGTH SOURCE_FILES FILE_COUNT)
message(DEBUG "Found ${FILE_COUNT} files to include:")

foreach(FILE_PATH ${SOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${SOURCE_DIR} ${FILE_PATH})
    message(DEBUG "  Processing: ${REL_PATH}")

    file(READ ${FILE_PATH} CONTENT)

    # Clean up content
    string(REGEX REPLACE "#pragma once[^\n]*\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#include[^\n]*\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE\n.*WASMDOM_INLINE.*#else.*#endif\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifndef WASMDOM_COVERAGE(.[^#]*)#endif\n?" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE.[^#]*#endif\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifndef WASMDOM_COVERAGE(.[^#]*)#else.[^#]*#endif\n?" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#ifdef WASMDOM_COVERAGE.[^#]*#else(.[^#]*)#endif\n?" "\\1" CONTENT "${CONTENT}")
    string(REGEX REPLACE "\n\n\n+" "\n\n" CONTENT "${CONTENT}")
    string(REGEX REPLACE "^[\n\r\t ]+" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "[\n\r\t ]+$" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "#define WASMDOM_SH_INLINE\n?" "" CONTENT "${CONTENT}")
    string(REGEX REPLACE "WASMDOM_SH_INLINE\n?" "inline " CONTENT "${CONTENT}")
    string(REGEX REPLACE "WASMDOM_INLINE\n?" "inline " CONTENT "${CONTENT}")

    # Add cleaned content
    file(APPEND ${OUTPUT_FILE} "// -----------------------------------------------------------------------------\n")
    file(APPEND ${OUTPUT_FILE} "// ${REL_PATH}\n")
    file(APPEND ${OUTPUT_FILE} "// -----------------------------------------------------------------------------\n")
    file(APPEND ${OUTPUT_FILE} "${CONTENT}")
    file(APPEND ${OUTPUT_FILE} "\n\n")
endforeach()

file(APPEND ${OUTPUT_FILE} "// End of single header library\n")
