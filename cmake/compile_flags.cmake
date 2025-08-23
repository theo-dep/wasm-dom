add_library(compile_flags INTERFACE)

target_compile_options(compile_flags INTERFACE
    -Werror -Wall -Wextra -pedantic
    -sSTRICT
    -Wno-dollar-in-identifier-extension
)
target_link_options(compile_flags INTERFACE -lembind)
