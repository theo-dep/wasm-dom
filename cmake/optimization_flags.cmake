add_library(optimization_flags INTERFACE)

target_compile_options(optimization_flags INTERFACE
    $<$<CONFIG:Release>:-O3>
)

if(NOT WASM_DOM_COVERAGE)
    target_link_options(optimization_flags INTERFACE
        -sNO_FILESYSTEM
    )
endif()

add_library(closure_optimization_flags INTERFACE)
target_link_libraries(closure_optimization_flags INTERFACE optimization_flags)

target_link_options(closure_optimization_flags INTERFACE
    --closure 1
)
