function(configure_html_file EXAMPLE_TARGET EXAMPLE_TITLE)
    set(CONFIGS Debug Release RelWithDebInfo MinSizeRel)

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
