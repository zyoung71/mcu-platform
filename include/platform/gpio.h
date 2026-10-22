#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    MCU_GPIO_INPUT,
    MCU_GPIO_OUTPUT
    
} mcu_gpio_dir_t;

typedef enum
{
    MCU_GPIO_PULL_NONE,
    MCU_GPIO_PULL_UP,
    MCU_GPIO_PULL_DOWN

} mcu_gpio_pull_t;

typedef enum 
{
    MCU_GPIO_IRQ_NONE      = 0,
    MCU_GPIO_IRQ_RISING    = 1u << 0,
    MCU_GPIO_IRQ_FALLING   = 1u << 1,
    MCU_GPIO_IRQ_HIGH      = 1u << 2,
    MCU_GPIO_IRQ_LOW       = 1u << 3

} mcu_gpio_irq_event_t;

typedef enum 
{
    MCU_GPIO_FUNC_SIO,
    MCU_GPIO_FUNC_SPI,
    MCU_GPIO_FUNC_UART,
    MCU_GPIO_FUNC_I2C,
    MCU_GPIO_FUNC_PWM,
    MCU_GPIO_FUNC_PIO,
    MCU_GPIO_FUNC_ADC

} mcu_gpio_func_t;

typedef void (*mcu_gpio_irq_callback_t)(uint8_t pin, uint32_t events, void* context);

void mcu_gpio_init(uint8_t pin);
void mcu_gpio_deinit(uint8_t pin);
void mcu_gpio_dir(uint8_t pin, mcu_gpio_dir_t dir);
void mcu_gpio_put(uint8_t pin, bool high);
bool mcu_gpio_get(uint8_t pin);
void mcu_gpio_pull(uint8_t pin, mcu_gpio_pull_t pull);
void mcu_gpio_func(uint8_t pin, mcu_gpio_func_t func);
void mcu_gpio_irq_set(uint8_t pin, uint32_t events, mcu_gpio_irq_callback_t cb, void* context);
void mcu_gpio_irq_enable(uint8_t pin, bool enabled);
void mcu_gpio_irq_clear(uint8_t pin);

#ifdef __cplusplus
}
#endif