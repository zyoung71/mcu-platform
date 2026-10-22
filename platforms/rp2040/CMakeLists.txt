if (NOT TARGET mcu-platform-rp2040)

    add_library(mcu-platform-rp2040 STATIC
        ${CMAKE_CURRENT_LIST_DIR}/time.c
        ${CMAKE_CURRENT_LIST_DIR}/gpio.c
        ${CMAKE_CURRENT_LIST_DIR}/spi.c
        ${CMAKE_CURRENT_LIST_DIR}/i2c.c
        ${CMAKE_CURRENT_LIST_DIR}/uart.c
        ${CMAKE_CURRENT_LIST_DIR}/pwm.c
        ${CMAKE_CURRENT_LIST_DIR}/adc.c
    )

    add_subdirectory(../../ build/mcu-platform-interface)

    target_link_libraries(mcu-platform-rp2040
        mcu-platform-interface

        pico_time
        pico_sync
        hardware_spi
        hardware_i2c
        hardware_uart
        hardware_pwm
        hardware_adc
        hardware_clocks
    )

endif()