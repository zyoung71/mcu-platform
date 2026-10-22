#include <hardware/platform_defs.h>
#include <pico/stdio.h>
#include <platform/time.h>
#include <platform/gpio.h>
#include <platform/spi.h>
#include <platform/i2c.h>
#include <platform/uart.h>
#include <platform/pwm.h>
#include <platform/adc.h>

int main()
{
    stdio_init_all();

    auto id = mcu_add_timer_periodic(1000000, [](void*){
        // callback func
    }, nullptr);

    mcu_gpio_init(0);
    mcu_gpio_dir(0, MCU_GPIO_INPUT);
    mcu_gpio_pull(0, MCU_GPIO_PULL_UP);
    mcu_gpio_irq_set(
        0,
        MCU_GPIO_IRQ_FALLING | MCU_GPIO_IRQ_RISING,
        [](uint8_t pin, uint32_t events, void* context){
            // callback
        },
    nullptr
    );
    mcu_gpio_irq_enable(0, true);

    mcu_timer_cancel(id);

    mcu_spi_bus_config_t cfg_spi = {
        .sck_pin = 2,
        .tx_pin = 3,
        .rx_pin = 4,
        .controller = 0,
        .baudrate = SYS_CLK_HZ / 4
    };

    mcu_spi_bus_t* bus_spi;
    mcu_spi_init(&cfg_spi, &bus_spi);

    mcu_i2c_bus_config_t cfg_i2c = {
        .sda_pin = 6,
        .scl_pin = 7,
        .controller = 1,
        .baudrate = 400000
    };

    mcu_i2c_bus_t* bus_i2c;
    mcu_i2c_init(&cfg_i2c, &bus_i2c);

    mcu_uart_bus_config_t cfg_uart = {
        .tx_pin = 0,
        .rx_pin = 1,
        .controller = 0,
        .baudrate = 9600,
        .data_bits = 8,
        .stop_bits = 1,
        .parity = MCU_UART_PARITY_NONE
    };

    mcu_uart_bus_t* bus_uart;
    mcu_uart_init(&cfg_uart, &bus_uart);

    mcu_pwm_config_t cfg_pwm = {
        .frequency_hz = 4000,
        .resolution_bits = 16,
        .inverted = false
    };

    mcu_pwm_init(0, 10, &cfg_pwm);

    mcu_adc_config_t cfg_adc = {
        .pin = 26,
        .resolution_bits = 12
    };

    mcu_adc_init(0, &cfg_adc);
}