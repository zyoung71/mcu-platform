#include "platform/spi.h"
#include "platform/util/util.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "pico/mutex.h"

typedef struct mcu_spi_bus
{
    spi_inst_t* instance;
    mutex_t mutex;
    bool initialized;
    
} rp_spi_bus_t;

static rp_spi_bus_t busses[NUM_SPIS];

__attribute__((constructor)) static void init_bus_slots()
{
    busses[0].instance = spi0;
    busses[0].initialized = false;
    mutex_init(&busses[0].mutex);

    busses[1].instance = spi1;
    busses[1].initialized = false;
    mutex_init(&busses[1].mutex);
}

bool mcu_spi_init(const mcu_spi_bus_config_t* cfg, mcu_spi_bus_t** bus)
{
    if (!cfg || !bus || cfg->controller >= NUM_SPIS || cfg->baudrate == 0)
        return false;

    rp_spi_bus_t* b = &busses[cfg->controller];
    if (b->initialized)
        return false;

    spi_init(b->instance, cfg->baudrate);
    
    if (is_gpio_invalid_or_unused(cfg->sck_pin))
        gpio_set_function(cfg->sck_pin, GPIO_FUNC_SPI);
    if (is_gpio_invalid_or_unused(cfg->tx_pin))
        gpio_set_function(cfg->tx_pin, GPIO_FUNC_SPI);
    if (is_gpio_invalid_or_unused(cfg->rx_pin))
        gpio_set_function(cfg->rx_pin, GPIO_FUNC_SPI);

    b->initialized = true;
    *bus = b;

    return true;
}

bool mcu_spi_deinit(mcu_spi_bus_t* bus)
{
    if (!bus || !bus->initialized)
        return false;

    mutex_enter_blocking(&bus->mutex);
    spi_deinit(bus->instance);

    bus->initialized = false;
    mutex_exit(&bus->mutex);
    return true;
}

bool mcu_spi_begin(mcu_spi_bus_t* bus, const mcu_spi_config_t* cfg)
{
    if (!bus || !bus->initialized || !cfg || cfg->baudrate == 0)
        return false;

    mutex_enter_blocking(&bus->mutex);

    uint clock_polarity = (cfg->mode >> 1) & 1;
    uint clock_phase = cfg->mode & 1;
    
    spi_set_baudrate(bus->instance, cfg->baudrate);
    spi_set_format(
        bus->instance,
        8,
        (spi_cpol_t)clock_polarity,
        (spi_cpha_t)clock_phase,
        cfg->order == MCU_SPI_MSB_FIRST ? SPI_MSB_FIRST : SPI_LSB_FIRST
    );
    return true;
}

bool mcu_spi_end(mcu_spi_bus_t* bus)
{
    if (bus && bus->initialized)
    {
        mutex_exit(&bus->mutex);
        return true;
    }
    return false;
}

bool mcu_spi_transfer(mcu_spi_bus_t* bus, const uint8_t* tx, uint8_t* rx, size_t len)
{
    if (!bus || !bus->initialized || (!tx && !rx && len != 0))
        return false;

    if (len == 0)
        return false;

    if (tx && rx)
        return spi_write_read_blocking(bus->instance, tx, rx, len) > 0;
    else if (tx)
        return spi_write_blocking(bus->instance, tx, len) > 0;
    else
        return spi_read_blocking(bus->instance, 0xFF, rx, len) > 0;
}

bool mcu_spi_read(mcu_spi_bus_t* bus, uint8_t* data, size_t len, uint8_t fill_byte)
{
    if (!bus || !bus->initialized || (!data && len != 0))
        return false;

    if (len == 0)
        return false;

    return spi_read_blocking(bus->instance, fill_byte, data, len) > 0;
}

bool mcu_spi_write(mcu_spi_bus_t* bus, const uint8_t* data, size_t len)
{
    if (!bus || !bus->initialized || (!data && len != 0))
        return false;

    if (len == 0)
        return false;

    return spi_write_blocking(bus->instance, data, len) > 0;
}