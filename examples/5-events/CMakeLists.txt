add_executable(events main.cpp)

target_link_libraries(events PRIVATE wasm-dom address_sanitizer_link compile_flags disable_exceptions optimization_flags)
target_link_options(events PRIVATE ${LINK_OPTIONS})

set(EXAMPLE_TARGET events.js PARENT_SCOPE)
set(EXAMPLE_TITLE Events PARENT_SCOPE)
