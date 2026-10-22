#include "platform/adc.h"
#include "hardware/adc.h"

#include <hardware/platform_defs.h>
#include <stddef.h>
#include <string.h>

typedef struct
{
    bool active;
    mcu_adc_t handle;
    uint8_t pin;
    uint8_t channel;

} rp_adc_handle_t;

static rp_adc_handle_t handles[NUM_ADC_CHANNELS];
static bool adc_initialized;

__attribute__((constructor)) static void init_handles()
{
    for (size_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        memset(&handles[i], 0, sizeof(rp_adc_handle_t));
    }
}

static rp_adc_handle_t* find_handle(mcu_adc_t adc)
{
    for (size_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        if (handles[i].active && handles[i].handle == adc)
            return &handles[i];
    }

    return NULL;
}

void mcu_adc_init(mcu_adc_t adc, const mcu_adc_config_t* cfg)
{
    if (find_handle(adc))
        return;

    for (size_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        if (handles[i].active && handles[i].pin == cfg->pin)
            return;
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
        return;

    if (!adc_initialized)
    {
        adc_init();
        adc_initialized = true;
    }

    adc_gpio_init(cfg->pin);

    slot->handle = adc;
    slot->pin = cfg->pin;
    slot->channel = cfg->pin - ADC_BASE_PIN;
    slot->active = true;
}

void mcu_adc_read(mcu_adc_t adc, uint16_t* value)
{
    if (!value)
        return;

    rp_adc_handle_t *handle = find_handle(adc);

    if (!handle)
        return;

    adc_select_input(handle->channel);
    *value = adc_read();
}

uint16_t mcu_adc_get_max_value(mcu_adc_t adc)
{
    if (!find_handle(adc))
        return 0;

    return 4095;
}

void mcu_adc_deinit(mcu_adc_t adc)
{
    rp_adc_handle_t *handle = find_handle(adc);

    if (handle == NULL)
        return;

    memset(handle, 0, sizeof(*handle));

    /*
     * Do not call adc_deinit() here: another registered
     * channel may still be using the shared peripheral.
     */
}