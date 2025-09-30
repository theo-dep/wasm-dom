set(EXAMPLE_DIRS
    0-console
    1-counter
    2-routing
    3-webcomponent
    4-webbenchmarks
    5-events
)

set(EXAMPLE_LIST "")
foreach(EXAMPLE_DIR ${EXAMPLE_DIRS})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${EXAMPLE_DIR})
    add_subdirectory(${EXAMPLE_DIR})

    set(EXAMPLE_LIST "${EXAMPLE_LIST}<li><a href=\"./${EXAMPLE_DIR}\">${EXAMPLE_TITLE}</a></li>")

    foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/example_index.html.in ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}/${EXAMPLE_DIR}/index.html @ONLY)
    endforeach()
endforeach()

foreach(CONFIG ${CMAKE_CONFIGURATION_TYPES})
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/index.html.in ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}/index.html @ONLY)
    file(COPY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/styles.css ${CMAKE_CURRENT_BINARY_DIR}/${CONFIG}/styles.css)
endforeach()
