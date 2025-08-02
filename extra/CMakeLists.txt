file(GLOB HEADER_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.hpp")
file(GLOB SOURCE_FILES "${CMAKE_SOURCE_DIR}/src/wasm-dom/*.cpp")

set(OUTPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/wasm-dom.hpp)

add_custom_command(
    OUTPUT ${OUTPUT_FILE}
    COMMAND ${CMAKE_COMMAND}
        -DPROJECT_NAME="${PROJECT_NAME}"
        -DHEADER_FILES="${HEADER_FILES}"
        -DSOURCE_FILES="${SOURCE_FILES}"
        -DOUTPUT_FILE="${OUTPUT_FILE}"
        -DCXX_COMPILER="${CMAKE_CXX_COMPILER}"
        -P ${CMAKE_CURRENT_SOURCE_DIR}/single_header.cmake
    DEPENDS ${HEADER_FILES} ${SOURCE_FILES}
    COMMENT "Generating single header file ${OUTPUT_FILE}"
)

add_custom_target(single_header ALL DEPENDS ${OUTPUT_FILE})
