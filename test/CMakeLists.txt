include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)
FetchContent_MakeAvailable(Catch2)

find_program(NPM_EXECUTABLE npm)
if(NOT NPM_EXECUTABLE)
    message(FATAL "npm not found")
endif()

message(CHECK_START "Installing test dependencies")
file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_CURRENT_BINARY_DIR}/package.json ONLY_IF_DIFFERENT)
file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json ${CMAKE_CURRENT_BINARY_DIR}/package-lock.json ONLY_IF_DIFFERENT)
execute_process(COMMAND ${NPM_EXECUTABLE} install WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_QUIET)
message(CHECK_PASS "done")

set(LINK_OPTIONS -sENVIRONMENT=node -sEXIT_RUNTIME -sNO_DISABLE_EXCEPTION_CATCHING
    $<$<CONFIG:Debug>:-gsource-map -fsanitize=address -sALLOW_MEMORY_GROWTH>
)

add_subdirectory(src)
add_subdirectory(benchmark)
add_subdirectory(single_header)
