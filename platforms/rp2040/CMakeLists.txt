if (NOT TARGET mcu-platform-rp2040)

    add_library(mcu-platform-rp2040 STATIC
        ${CMAKE_CURRENT_LIST_DIR}/time.c
        ${CMAKE_CURRENT_LIST_DIR}/gpio.c
        ${CMAKE_CURRENT_LIST_DIR}/spi.c
        ${CMAKE_CURRENT_LIST_DIR}/i2c.c
        ${CMAKE_CURRENT_LIST_DIR}/uart.c
        ${CMAKE_CURRENT_LIST_DIR}/pwm.c
        ${CMAKE_CURRENT_LIST_DIR}/adc.c
        ${CMAKE_CURRENT_LIST_DIR}/util/queue.c
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

    set(PLATFORM_NAME "rp2040")
    set(NUM_GPIO 30)
    set(NUM_SPI 2)
    set(NUM_I2C 2)
    set(NUM_UART 2)
    set(NUM_ADC 4)
    set(NUM_PWM_SLICES 8)
    set(NUM_PWM_CHANNELS 16)
    set(NUM_TIMERS 4)
    set(NUM_PIO 2)

    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/../../include/platform/platform_config.h.in"
        "${PROJECT_BINARY_DIR}/generated/platform/platform_config.h"
        @ONLY
    )

endif()