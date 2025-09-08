function(configure_html_file EXAMPLE_TARGET EXAMPLE_TITLE)
    set(CONFIGS Debug Release RelWithDebInfo MinSizeRel)

    get_filename_component(EXAMPLE_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(EXAMPLE_REF https://github.com/theo-dep/wasm-dom/blob/main/examples/${EXAMPLE_DIR}/main.cpp)

    foreach(config ${CONFIGS})
        set(EXAMPLE_TARGET_PATH ${config}/${EXAMPLE_TARGET})

        configure_file(
            ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/index.html.in
            ${CMAKE_CURRENT_BINARY_DIR}/${config}/index.html
            @ONLY
        )

        file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${config})
    endforeach()
endfunction()
