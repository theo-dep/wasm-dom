add_library(address_sanitizer_compile INTERFACE)

if(WASM_DOM_ASAN)
    if(MSVC)
        target_compile_options(address_sanitizer_compile INTERFACE
            $<$<CONFIG:Debug>:/fsanitize=address>
        )
    else()
        target_compile_options(address_sanitizer_compile INTERFACE
            $<$<CONFIG:Debug>:-fsanitize=address>
        )
    endif()

    if(EMSCRIPTEN)
        target_compile_options(address_sanitizer_compile INTERFACE
            $<$<CONFIG:Debug>:-gsource-map>
        )
    endif()
endif()

add_library(address_sanitizer_link INTERFACE)

if(WASM_DOM_ASAN)
    if(MSVC)
        target_compile_options(address_sanitizer_compile INTERFACE
            $<$<CONFIG:Debug>:/fsanitize=address>
        )
    else()
        target_link_options(address_sanitizer_link INTERFACE
            $<$<CONFIG:Debug>:-fsanitize=address>
        )
    endif()

    if(EMSCRIPTEN)
        target_link_options(address_sanitizer_link INTERFACE
            $<$<CONFIG:Debug>:-sALLOW_MEMORY_GROWTH>
        )
    endif()
endif()
