#include "platform/adc.h"
#include "hardware/adc.h"

#include <stddef.h>
#include <string.h>

typedef struct
{
    bool active;
    uint8_t pin;
    uint8_t channel;

} rp_adc_handle_t;

static rp_adc_handle_t handles[NUM_ADC_CHANNELS] = {0};
static bool adc_initialized = false;

static rp_adc_handle_t* find_handle(uint8_t adc_pin)
{
    for (size_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        if (handles[i].active && handles[i].pin == adc_pin)
            return &handles[i];
    }

    return NULL;
}

bool mcu_adc_init(uint8_t adc_pin)
{
    if (find_handle(adc_pin))
        return false;

    for (size_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        if (handles[i].active && handles[i].pin == adc_pin)
            return false;
    }

    rp_adc_handle_t* slot = NULL;

    for (size_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        if (!handles[i].active)
        {
            slot = &handles[i];
            break;
        }
    }

    if (!slot)
        return false;

    if (!adc_initialized)
    {
        adc_init();
        adc_initialized = true;
    }

    adc_gpio_init(adc_pin);

    slot->pin = adc_pin;
    slot->channel = adc_pin - ADC_BASE_PIN;
    slot->active = true;

    return true;
}

uint32_t mcu_adc_read(uint8_t adc)
{
    rp_adc_handle_t* handle = find_handle(adc);

    if (!handle)
        return 0;

    adc_select_input(handle->channel);
    return (uint32_t)adc_read();
}

uint16_t mcu_adc_get_max_value(uint8_t adc)
{
    if (!find_handle(adc))
        return 0;

    return 4095;
}

bool mcu_adc_deinit(uint8_t adc)
{
    rp_adc_handle_t *handle = find_handle(adc);

    if (!handle)
        return false;

    memset(handle, 0, sizeof(*handle));
    return true;
}