#include <hardware/irq.h>
#include <pico/stdio.h>
#include <pico/stdlib.h>
#include <stdio.h>
#include <platform/time.h>
#include <platform/gpio.h>
#include <platform/spi.h>
#include <platform/i2c.h>
#include <platform/uart.h>
#include <platform/pwm.h>
#include <platform/adc.h>

#include <platform/util/queue.h>

bool flag = false;

void timer_cb(void*) {}

void gpio_cb(uint8_t pin, uint32_t events, void* context)
{
    flag = true;
}

int main()
{
    stdio_init_all();

    mcu_timer_id_t id = mcu_add_timer_periodic(1000000, &timer_cb, NULL);

    mcu_gpio_init(15);
    mcu_gpio_dir(15, MCU_GPIO_INPUT);
    mcu_gpio_pull(15, MCU_GPIO_PULL_UP);
    mcu_gpio_irq_set(15, MCU_GPIO_IRQ_FALLING | MCU_GPIO_IRQ_RISING, &gpio_cb, NULL);
    mcu_gpio_irq_enable(15, true);

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

    mcu_pwm_init(8, 1000, false);
    mcu_pwm_set_duty(8, 0.5f);
    mcu_pwm_set_enabled(8, true);

    mcu_adc_init(26);

    mcu_queue_t* queue;
    mcu_queue_init(&queue, sizeof(int32_t), 8);

    int32_t number = 50;
    mcu_queue_try_push(queue, &number);

    while (1)
    {
        if (flag)
        {
            flag = false;
            printf("Flag caught. ADC: %d\n", mcu_adc_read(26));
        }

        sleep_ms(500);
        printf("Hello World.\n");
    }
}