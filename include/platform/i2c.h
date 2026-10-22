#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct mcu_i2c_bus mcu_i2c_bus_t;

typedef struct
{
    uint8_t sda_pin;
    uint8_t scl_pin;
    uint8_t controller;
    uint32_t baudrate;

} mcu_i2c_bus_config_t;

void mcu_i2c_init(const mcu_i2c_bus_config_t* cfg, mcu_i2c_bus_t** bus);
void mcu_i2c_deinit(mcu_i2c_bus_t* bus);
void mcu_i2c_begin(mcu_i2c_bus_t* bus, const mcu_i2c_bus_config_t* cfg);
void mcu_i2c_end(mcu_i2c_bus_t* bus);
void mcu_i2c_read(mcu_i2c_bus_t* bus, uint8_t addr, uint8_t* data, size_t len, bool no_stop);
void mcu_i2c_write(mcu_i2c_bus_t* bus, uint8_t addr, const uint8_t* data, size_t len, bool no_stop);
void mcu_i2c_write_read(mcu_i2c_bus_t* bus, uint8_t addr, const uint8_t* tx, size_t tx_len, uint8_t* rx, size_t rx_len);

#ifdef __cplusplus
}
#endif