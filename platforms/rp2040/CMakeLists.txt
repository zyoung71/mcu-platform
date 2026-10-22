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

    target_compile_definitions(mcu-platform-rp2040 PUBLIC
        MCU_PLATFORM_NAME="rp2040"

        MCU_NUM_GPIO=30
        MCU_NUM_CPU_CORES=2
        MCU_NUM_SPI=2
        MCU_NUM_I2C=2
        MCU_NUM_UART=2
        MCU_NUM_PWM_SLICE=8
        MCU_NUM_PWM_CHANNELS=16
        MCU_NUM_ADC=4
        MCU_NUM_TIMERS=4
        MCU_NUM_PIO=2
    )

endif()