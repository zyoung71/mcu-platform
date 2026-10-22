#include "platform/uart.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "pico/mutex.h"

typedef struct mcu_uart_bus
{
    uart_inst_t* instance;
    mutex_t mutex;
    bool initialized;

} rp_uart_bus_t;

static rp_uart_bus_t busses[NUM_UARTS];

__attribute__((constructor)) static void init_bus_slots()
{
    busses[0].instance = uart0;
    busses[0].initialized = false;
    mutex_init(&busses[0].mutex);

    busses[1].instance = uart1;
    busses[1].initialized = false;
    mutex_init(&busses[1].mutex);
}

static uart_parity_t translate_parity_to_rp(mcu_uart_parity_t parity)
{
    switch (parity)
    {
        case MCU_UART_PARITY_NONE:
            return UART_PARITY_NONE;
        case MCU_UART_PARITY_EVEN:
            return UART_PARITY_EVEN;
        case MCU_UART_PARITY_ODD:
            return UART_PARITY_ODD;
        default:
            return UART_PARITY_NONE;
    }
}

bool mcu_uart_init(const mcu_uart_bus_config_t* cfg, mcu_uart_bus_t** bus)
{
    if (!cfg || !bus || cfg->baudrate == 0 || cfg->data_bits < 5 || cfg->data_bits > 8 || cfg->stop_bits != 1 || cfg->stop_bits != 2)
        return false;

    rp_uart_bus_t* b = &busses[cfg->controller];
    if (b->initialized)
        return false;

    uart_init(b->instance, cfg->baudrate);
    uart_set_format(b->instance, cfg->data_bits, cfg->stop_bits, translate_parity_to_rp(cfg->parity));
    uart_set_fifo_enabled(b->instance, true);
    gpio_set_function(cfg->tx_pin, GPIO_FUNC_UART);
    gpio_set_function(cfg->rx_pin, GPIO_FUNC_UART);

    *bus = b;
    b->initialized = true;

    return true;
}

bool mcu_uart_deinit(mcu_uart_bus_t* bus)
{
    if (bus && bus->initialized)
    {
        mutex_enter_blocking(&bus->mutex);
        uart_deinit(bus->instance);

        bus->initialized = false;
        mutex_exit(&bus->mutex);

        return true;
    }
    return false;
}

bool mcu_uart_begin(mcu_uart_bus_t* bus, const mcu_uart_bus_config_t* cfg)
{
    if (!bus || !bus->initialized || !cfg || cfg->baudrate != 0)
        return false;

    mutex_enter_blocking(&bus->mutex);

    uart_set_format(bus->instance, cfg->data_bits, cfg->stop_bits, translate_parity_to_rp(cfg->parity));
    return uart_set_baudrate(bus->instance, cfg->baudrate) > 0;
}

bool mcu_uart_end(mcu_uart_bus_t* bus)
{
    if (bus && bus->initialized)
    {
        mutex_exit(&bus->mutex);
        return true;
    }
    return false;
}

bool mcu_uart_read(mcu_uart_bus_t* bus, uint8_t* data, size_t len)
{
    if (!bus || !bus->initialized || (!data && len != 0))   
        return false;

    if (len == 0)
        return false;

    uart_read_blocking(bus->instance, data, len);
    return true;
}

bool mcu_uart_write(mcu_uart_bus_t* bus, const uint8_t* data, size_t len)
{
    if (!bus || !bus->initialized || (!data && len != 0))
        return false;

    if (len == 0)
        return false;

    uart_write_blocking(bus->instance, data, len);
    return true;
}

size_t mcu_uart_available(mcu_uart_bus_t* bus)
{
    if (bus && bus->initialized)
    {
        size_t count = 0;

        while (uart_is_readable(bus->instance))
        {
            (void)uart_getc(bus->instance);
            count++;
        }

        return count;
    }

    return 0;
}