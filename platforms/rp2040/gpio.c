#include "platform/gpio.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

void mcu_gpio_init(uint8_t pin)
{
    gpio_init(pin);
}

void mcu_gpio_deinit(uint8_t pin)
{
    gpio_deinit(pin);
}

void mcu_gpio_dir(uint8_t pin, mcu_gpio_dir_t dir)
{
    gpio_set_dir(pin, dir);
}

void mcu_gpio_put(uint8_t pin, bool high)
{
    gpio_put(pin, high);
}

bool mcu_gpio_get(uint8_t pin)
{
    return gpio_get(pin);
}

void mcu_gpio_pull(uint8_t pin, mcu_gpio_pull_t pull)
{
    gpio_set_pulls(pin, pull == MCU_GPIO_PULL_UP, pull == MCU_GPIO_PULL_DOWN);
}

void mcu_gpio_func(uint8_t pin, mcu_gpio_func_t func)
{
    gpio_function_t rp_func;
    switch (func)
    {
        case MCU_GPIO_FUNC_SIO:
            rp_func = GPIO_FUNC_SIO; break;
        case MCU_GPIO_FUNC_SPI:
            rp_func = GPIO_FUNC_SPI; break;
        case MCU_GPIO_FUNC_UART:
            rp_func = GPIO_FUNC_UART; break;
        case MCU_GPIO_FUNC_I2C:
            rp_func = GPIO_FUNC_I2C; break;
        case MCU_GPIO_FUNC_PWM:
            rp_func = GPIO_FUNC_PWM; break;
        case MCU_GPIO_FUNC_PIO:
            rp_func = GPIO_FUNC_PIO0; break;
        case MCU_GPIO_FUNC_ADC:
            adc_gpio_init(pin); return;
        default:
            return;
    }

    gpio_set_function(pin, rp_func);
}

typedef struct
{
    uint32_t events;
    mcu_gpio_irq_callback_t cb;
    void* context;
    bool registered;
    bool enabled;
    
} gpio_irq_entry_t;

static gpio_irq_entry_t irq_entries[NUM_BANK0_GPIOS];

static uint32_t translate_event_to_rp(uint32_t events)
{
    uint32_t ret = 0;

    if (events & MCU_GPIO_IRQ_RISING)
        ret |= GPIO_IRQ_EDGE_RISE;
    if (events & MCU_GPIO_IRQ_FALLING)
        ret |= GPIO_IRQ_EDGE_FALL;
    if (events & MCU_GPIO_IRQ_HIGH)
        ret |= GPIO_IRQ_LEVEL_HIGH;
    if (events & MCU_GPIO_IRQ_LOW)
        ret |= GPIO_IRQ_LEVEL_LOW;

    return ret;
}

static uint32_t translate_event_from_rp(uint32_t events)
{
    uint32_t ret = MCU_GPIO_IRQ_NONE;

    if (events & GPIO_IRQ_EDGE_RISE)
        ret |= MCU_GPIO_IRQ_RISING;
    if (events & GPIO_IRQ_EDGE_FALL)
        ret |= MCU_GPIO_IRQ_FALLING;
    if (events & GPIO_IRQ_LEVEL_HIGH)
        ret |= MCU_GPIO_IRQ_HIGH;
    if (events & GPIO_IRQ_LEVEL_LOW)
        ret |= MCU_GPIO_IRQ_LOW;

    return ret;
}

static void gpio_irq_dispatch(uint pin, uint32_t events)
{
    gpio_irq_entry_t* entry = &irq_entries[pin];

    if (!entry->registered || !entry->enabled || !entry->cb)
        return;

    uint32_t rp_events = translate_event_from_rp(events);
    if (rp_events != MCU_GPIO_IRQ_NONE)
        entry->cb((uint8_t)pin, rp_events, entry->context);
}

__attribute__((constructor)) static void gpio_irq_init()
{
    irq_set_enabled(IO_IRQ_BANK0, true);
    gpio_set_irq_callback(&gpio_irq_dispatch);
}

void mcu_gpio_irq_set(uint8_t pin, uint32_t events, mcu_gpio_irq_callback_t cb, void* context)
{
    const uint32_t valid = 0x0F; // OR logic on all irq events

    if ((events & ~valid) != 0)
        return;

    if (events != MCU_GPIO_IRQ_NONE && !cb)
        return;

    gpio_irq_entry_t* entry = &irq_entries[pin];

    gpio_set_irq_enabled(pin, translate_event_to_rp(events), false);

    entry->enabled = true;
    entry->events = events;
    entry->cb = cb;
    entry->registered = (events != MCU_GPIO_IRQ_NONE);
}

void mcu_gpio_irq_enable(uint8_t pin, bool enabled)
{
    gpio_irq_entry_t* entry = &irq_entries[pin];

    if (entry->registered)
    {
        gpio_set_irq_enabled(pin, translate_event_to_rp(entry->events), enabled);
        entry->enabled = enabled;
    }
}

void mcu_gpio_irq_clear(uint8_t pin)
{
    gpio_irq_entry_t* entry = &irq_entries[pin];

    gpio_set_irq_enabled(pin, translate_event_to_rp(entry->enabled), false);

    entry->enabled = false;
    entry->registered = false;
    entry->events = MCU_GPIO_IRQ_NONE;
    entry->cb = NULL;
    entry->context = NULL;
}