if (NOT TARGET mcu-platform-interface)

    add_library(mcu-platform-interface INTERFACE)

    target_include_directories(mcu-platform-interface INTERFACE
        include
    )

endif()