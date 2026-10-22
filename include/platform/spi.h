#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct mcu_spi_bus mcu_spi_bus_t;

typedef enum
{
    MCU_SPI_MODE_0,
    MCU_SPI_MODE_1,
    MCU_SPI_MODE_2,
    MCU_SPI_MODE_3

} mcu_spi_mode_t;

typedef enum
{
    MCU_SPI_MSB_FIRST,
    MCU_SPI_LSB_FIRST

} mcu_spi_order_t;

typedef struct
{
    uint32_t baudrate;
    mcu_spi_mode_t mode;
    mcu_spi_order_t order;

} mcu_spi_config_t;

typedef struct
{
    uint8_t sck_pin;
    uint8_t tx_pin;
    uint8_t rx_pin;
    uint8_t controller;
    uint32_t baudrate;
    
} mcu_spi_bus_config_t;

void mcu_spi_init(const mcu_spi_bus_config_t* cfg, mcu_spi_bus_t** bus);
void mcu_spi_deinit(mcu_spi_bus_t* bus);
void mcu_spi_begin(mcu_spi_bus_t* bus, const mcu_spi_config_t* cfg);
void mcu_spi_end(mcu_spi_bus_t* bus);
void mcu_spi_transfer(mcu_spi_bus_t* bus, const uint8_t* tx, uint8_t* rx, size_t len);
void mcu_spi_read(mcu_spi_bus_t* bus, uint8_t* data, size_t len, uint8_t fill_byte);
void mcu_spi_write(mcu_spi_bus_t* bus, const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif