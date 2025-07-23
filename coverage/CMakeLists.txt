set(NCNN_SIMPLEGCOV ON)
configure_file(platform.h.in platform.h)

set(TARGET_NAME ncnn-gcov)

add_library(${TARGET_NAME} STATIC
    simplegcov.cpp
)

include(GenerateExportHeader)
generate_export_header(${TARGET_NAME} BASE_NAME ncnn)

target_compile_definitions(${TARGET_NAME} PUBLIC NCNN_STATIC_DEFINE)
target_include_directories(${TARGET_NAME} PUBLIC . ${CMAKE_CURRENT_BINARY_DIR})
target_compile_options(${TARGET_NAME} PUBLIC -Wno-unused-parameter --coverage -fprofile-arcs -ftest-coverage)
target_link_libraries(${TARGET_NAME} PUBLIC -sFORCE_FILESYSTEM -lnodefs.js -lnoderawfs.js)
