add_library(common STATIC
    catch_emscripten.cpp
    jsdom.cpp
)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(common PUBLIC -Wno-c2y-extensions)
target_link_libraries(common PRIVATE embind Catch2::Catch2)
