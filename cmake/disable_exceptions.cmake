add_library(disable_exceptions INTERFACE)

target_compile_options(disable_exceptions INTERFACE
    -sDISABLE_EXCEPTION_CATCHING
    -fno-exceptions
)
target_link_options(disable_exceptions INTERFACE
    -sDISABLE_EXCEPTION_CATCHING
)
