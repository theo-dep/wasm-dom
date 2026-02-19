add_library(optimization_flags INTERFACE)

if(WASM_DOM_FULL_OPTIMISATION)
    if(EMSCRIPTEN)
        target_compile_definitions(optimization_flags INTERFACE
            EMSCRIPTEN_HAS_UNBOUND_TYPE_NAMES=0
        )
    endif()

    if(MSVC)
        target_compile_options(optimization_flags INTERFACE
            $<$<CONFIG:Release>:/Ox>
            /GR-
        )
        target_link_options(optimization_flags INTERFACE
            /GR-
        )
    else()
        target_compile_options(optimization_flags INTERFACE
            $<$<CONFIG:Release>:-O3>
            -fno-rtti
        )
        target_link_options(optimization_flags INTERFACE
            -fno-rtti
        )
    endif()

    if(EMSCRIPTEN)
        target_link_options(optimization_flags INTERFACE
            $<$<CONFIG:Release>:--closure 1>
        )
    endif()

    if(NOT WASM_DOM_COVERAGE AND EMSCRIPTEN)
        target_link_options(optimization_flags INTERFACE
            -sNO_FILESYSTEM
        )
    endif()
endif()
