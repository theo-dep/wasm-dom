add_library(address_sanitizer_compile INTERFACE)

if(WASM_DOM_ASAN)
    target_compile_options(address_sanitizer_compile INTERFACE
        $<$<CONFIG:Debug>:-gsource-map -fsanitize=address>
    )
endif()

add_library(address_sanitizer_link INTERFACE)

if(WASM_DOM_ASAN)
    target_link_options(address_sanitizer_link INTERFACE
        $<$<CONFIG:Debug>:-fsanitize=address -sALLOW_MEMORY_GROWTH>
    )
endif()
