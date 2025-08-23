add_library(optimization_flags INTERFACE)

target_compile_options(optimization_flags INTERFACE
    $<$<CONFIG:Release>:-O3>
)
target_link_options(optimization_flags INTERFACE
    $<$<CONFIG:Release>:--closure 1>
)

if(NOT WASM_DOM_COVERAGE)
    target_link_options(optimization_flags INTERFACE
        -sNO_FILESYSTEM
    )
endif()
