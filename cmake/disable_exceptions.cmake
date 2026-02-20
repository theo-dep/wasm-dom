add_library(disable_exceptions INTERFACE)

if(WASM_DOM_NO_EXCEPTION)
    if(MSVC)
        target_compile_options(disable_exceptions INTERFACE
            /EHs-c-
        )
    else()
        target_compile_options(disable_exceptions INTERFACE
            -fno-exceptions
        )
    endif()

    if(EMSCRIPTEN)
        target_compile_options(disable_exceptions INTERFACE
            -sDISABLE_EXCEPTION_CATCHING
        )
        target_link_options(disable_exceptions INTERFACE
            -sDISABLE_EXCEPTION_CATCHING
        )
    endif()
endif()
