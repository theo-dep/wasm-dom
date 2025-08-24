add_library(optimization_flags INTERFACE)

target_compile_definitions(optimization_flags INTERFACE
    EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0
)
target_compile_options(optimization_flags INTERFACE
    $<$<CONFIG:Release>:-O3>
    -fno-rtti
)
target_link_options(optimization_flags INTERFACE
    $<$<CONFIG:Release>:--closure 1>
    -fno-rtti
)

if(NOT WASM_DOM_COVERAGE)
    target_link_options(optimization_flags INTERFACE
        -sNO_FILESYSTEM
    )
endif()
