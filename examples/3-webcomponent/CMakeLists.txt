add_executable(web-component main.cpp)

target_link_libraries(web-component PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags)
target_compile_options(web-component PRIVATE -Wno-dollar-in-identifier-extension)
target_link_options(web-component PRIVATE -sENVIRONMENT=web)

set(EXAMPLE_TARGET web-component.js PARENT_SCOPE)
set(EXAMPLE_TITLE WebComponent PARENT_SCOPE)
