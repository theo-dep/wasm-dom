project(llvm-gcov LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} STATIC
    GCDAProfiling.c InstrProfilingUtil.c
)

target_compile_options(${PROJECT_NAME} PUBLIC -g -O0 -Wno-unused-parameter --coverage -fprofile-arcs -ftest-coverage)
target_link_libraries(${PROJECT_NAME} PUBLIC -sFORCE_FILESYSTEM -lnodefs.js -lnoderawfs.js)
