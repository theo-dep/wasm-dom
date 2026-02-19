add_library(compile_flags INTERFACE)

if(MSVC)
    target_compile_options(compile_flags INTERFACE
        /Zc:preprocessor
    )
endif()

if(WASM_DOM_STRICT)
    if(MSVC)
        target_compile_options(compile_flags INTERFACE
            /W4 /WX
        )
    else()
        target_compile_options(compile_flags INTERFACE
            -Werror -Wall -Wextra -pedantic
        )
    endif()

    if(EMSCRIPTEN)
        target_compile_options(compile_flags INTERFACE
            -sSTRICT
        )
    endif()
endif()
