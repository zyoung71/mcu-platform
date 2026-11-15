if (NOT TARGET mcu-platform-interface)

    add_library(mcu-platform-interface INTERFACE)

    target_sources(mcu-platform-interface INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/util/queue.c
    )

    target_include_directories(mcu-platform-interface INTERFACE
        include
        ${PROJECT_BINARY_DIR}/generated
    )

endif()