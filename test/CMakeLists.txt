include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.10.0
)
FetchContent_MakeAvailable(Catch2)

target_link_options(Catch2WithMain PUBLIC -sENVIRONMENT=node -sEXIT_RUNTIME -sNO_DISABLE_EXCEPTION_CATCHING)

find_program(NPM_EXECUTABLE npm REQUIRED)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    set(NPM_EXECUTABLE ${NPM_EXECUTABLE}.cmd)
endif()

message(CHECK_START "Installing test dependencies")
file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/package.json ${CMAKE_CURRENT_BINARY_DIR}/package.json ONLY_IF_DIFFERENT)
file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/package-lock.json ${CMAKE_CURRENT_BINARY_DIR}/package-lock.json ONLY_IF_DIFFERENT)
execute_process(COMMAND ${NPM_EXECUTABLE} install WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_QUIET COMMAND_ERROR_IS_FATAL ANY)
message(CHECK_PASS "done")

add_subdirectory(common)
add_subdirectory(src)
add_subdirectory(benchmark)
add_subdirectory(single_header)
