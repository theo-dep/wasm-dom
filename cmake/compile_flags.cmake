add_library(compile_flags INTERFACE)

if(WASM_DOM_STRICT)
    target_compile_options(compile_flags INTERFACE
        -Werror -Wall -Wextra -pedantic
        -sSTRICT
    )
endif()
