#include "platform/i2c.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/mutex.h"
#include <pico/error.h>

typedef struct mcu_i2c_bus
{
    i2c_inst_t* instance;
    mutex_t mutex;
    bool initialized;

} rp_i2c_bus_t;

static rp_i2c_bus_t busses[NUM_I2CS];

__attribute__((constructor)) static void init_bus_slots()
{
    busses[0].instance = i2c0;
    busses[0].initialized = false;
    mutex_init(&busses[0].mutex);

    busses[1].instance = i2c1;
    busses[1].initialized = false;
    mutex_init(&busses[1].mutex);
}

bool mcu_i2c_init(const mcu_i2c_bus_config_t* cfg, mcu_i2c_bus_t** bus)
{
    if (!cfg || !bus || cfg->baudrate == 0)
        return false;

    rp_i2c_bus_t* b = &busses[cfg->controller];
    if (b->initialized)
        false;

    uint brate = i2c_init(b->instance, cfg->baudrate);
    gpio_set_function(cfg->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(cfg->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(cfg->sda_pin);
    gpio_pull_up(cfg->scl_pin);

    *bus = b;
    b->initialized = true;

    return brate > 0;
}

bool mcu_i2c_deinit(mcu_i2c_bus_t* bus)
{
    if (bus && bus->initialized)
    {
        mutex_enter_blocking(&bus->mutex);
        i2c_deinit(bus->instance);

        bus->initialized = false;
        mutex_exit(&bus->mutex);

        return true;
    }
    return false;
}

bool mcu_i2c_begin(mcu_i2c_bus_t* bus, const mcu_i2c_bus_config_t* cfg)
{
    if (!bus || !bus->initialized || !cfg || cfg->baudrate == 0)
        return false;

    mutex_enter_blocking(&bus->mutex);

    return i2c_set_baudrate(bus->instance, cfg->baudrate) > 0;
}

bool mcu_i2c_end(mcu_i2c_bus_t* bus)
{
    if (bus && bus->initialized)
    {
        mutex_exit(&bus->mutex);
        return true;
    }
    return false;
}

bool mcu_i2c_read(mcu_i2c_bus_t* bus, uint8_t addr, uint8_t* data, size_t len, bool no_stop)
{
    if (!bus || !bus->initialized || (!data && len != 0) || addr > 0x7F)
        return false;

    if (len == 0)
        return false;

    return i2c_read_blocking(bus->instance, addr, data, len, no_stop) != PICO_ERROR_GENERIC;
}

bool mcu_i2c_write(mcu_i2c_bus_t* bus, uint8_t addr, const uint8_t* data, size_t len, bool no_stop)
{
    if (!bus || !bus->initialized || (!data && len != 0) || addr > 0x7F)
        return false;

    if (len == 0)
        return false;

    return i2c_write_blocking(bus->instance, addr, data, len, no_stop) != PICO_ERROR_GENERIC;
}

bool mcu_i2c_write_read(mcu_i2c_bus_t* bus, uint8_t addr, const uint8_t* tx, size_t tx_len, uint8_t* rx, size_t rx_len)
{
    if (!bus || !bus->initialized || addr > 0x7F || !tx || !rx || tx_len == 0 || rx_len == 0)
        return false;

    int bytes_written = i2c_write_blocking(bus->instance, addr, tx, tx_len, true);
    if (bytes_written >= 0 && (size_t)bytes_written == tx_len)
        return i2c_read_blocking(bus->instance, addr, rx, rx_len, true) != PICO_ERROR_GENERIC;
    else
        return false;
}