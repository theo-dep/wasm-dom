Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.8.1
)

FetchContent_MakeAvailable(Catch2)

add_executable(tests
    tohtml.cpp
)

target_link_libraries(tests PRIVATE wasm-dom Catch2::Catch2WithMain)

add_test(NAME toHTML
    COMMAND ${NODE_JS_EXECUTABLE} "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/tests.js" [toHTML]
)
