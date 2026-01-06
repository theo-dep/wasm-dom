add_library(common STATIC
    catch_emscripten.cpp
    jsdom.cpp
)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common PRIVATE embind Catch2::Catch2)
