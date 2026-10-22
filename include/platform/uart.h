#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct mcu_uart_bus mcu_uart_bus_t;

typedef enum
{
    MCU_UART_PARITY_NONE,
    MCU_UART_PARITY_EVEN,
    MCU_UART_PARITY_ODD

} mcu_uart_parity_t;

typedef struct
{
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint8_t controller;
    
    uint32_t baudrate;

    uint8_t data_bits;
    uint8_t stop_bits;
    mcu_uart_parity_t parity;

} mcu_uart_bus_config_t;

void mcu_uart_init(const mcu_uart_bus_config_t* cfg, mcu_uart_bus_t** bus);
void mcu_uart_deinit(mcu_uart_bus_t* bus);
void mcu_uart_begin(mcu_uart_bus_t* bus, const mcu_uart_bus_config_t* cfg);
void mcu_uart_end(mcu_uart_bus_t* bus);
void mcu_uart_read(mcu_uart_bus_t* bus, uint8_t* data, size_t len);
void mcu_uart_write(mcu_uart_bus_t* bus, const uint8_t* data, size_t len);
size_t mcu_uart_available(mcu_uart_bus_t* bus);

#ifdef __cplusplus
}
#endif