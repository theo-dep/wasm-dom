add_library(address_sanitizer_compile INTERFACE)

target_compile_options(address_sanitizer_compile INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${WASM_DOM_ASAN}>>:-gsource-map -fsanitize=address>
)

add_library(address_sanitizer_link INTERFACE)

target_link_options(address_sanitizer_link INTERFACE
    $<$<AND:$<CONFIG:Debug>,$<BOOL:${WASM_DOM_ASAN}>>:-fsanitize=address -sALLOW_MEMORY_GROWTH>
)
