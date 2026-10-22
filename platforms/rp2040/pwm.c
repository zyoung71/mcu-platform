#include "platform/pwm.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include <hardware/platform_defs.h>
#include <string.h>

#ifdef PICO_RP2040
#define NUM_PWM_CHANNELS_PER_SLICE 2
#elif defined(PICO_RP2350)
#define NUM_PWM_CHANNELS_PER_SLICE 2
#endif

typedef struct
{
    uint16_t wrap;
    bool configured;
    uint8_t resolution_bits;
    uint32_t frequency_hz;
    float divider;

    bool channel_active[NUM_PWM_CHANNELS_PER_SLICE];
    bool channel_inverted[NUM_PWM_CHANNELS_PER_SLICE];
    uint8_t channel_pin[NUM_PWM_CHANNELS_PER_SLICE];

} rp_pwm_slice_t;

typedef struct
{
    mcu_pwm_t pwm;
    bool active;
    uint8_t pin;
    uint8_t slice;
    uint8_t channel;
    uint16_t max_duty;

} rp_pwm_handle_t;

static rp_pwm_slice_t slices[NUM_PWM_SLICES];
static rp_pwm_handle_t handles[NUM_PWM_CHANNELS_PER_SLICE * NUM_PWM_SLICES];

__attribute__((constructor)) static void init_slices_and_handles()
{
    for (size_t i = 0; i < NUM_PWM_SLICES; i++)
    {
        memset(&slices[i], 0, sizeof(rp_pwm_slice_t));
    }
    for (size_t i = 0; i < NUM_PWM_CHANNELS_PER_SLICE * NUM_PWM_SLICES; i++)
    {
        memset(&handles[i], 0, sizeof(rp_pwm_handle_t));
    }
}

static rp_pwm_handle_t* find_handle(mcu_pwm_t pwm)
{
    for (size_t i = 0; i < NUM_PWM_CHANNELS_PER_SLICE * NUM_PWM_SLICES; i++)
    {
        if (handles[i].active && handles[i].pwm == pwm)
            return &handles[i];
    }
    return NULL;
}

static rp_pwm_handle_t* find_free_handle(void)
{
    for (size_t i = 0; i < NUM_PWM_CHANNELS_PER_SLICE * NUM_PWM_SLICES; i++) {
        if (!handles[i].active)
            return &handles[i];
    }
    return NULL;
}

static bool calculate_pwm(uint32_t frequency_hz, uint8_t resolution_bits, uint16_t* wrap, float* divider)
{
    if (frequency_hz == 0 || resolution_bits < 2 || resolution_bits > 16 || !wrap || !divider)
        return false;

    uint32_t clock_hz = clock_get_hz(clk_sys);
    uint32_t max_duty = (1u << resolution_bits) - 1u;
    uint32_t top = max_duty - 1u;

    float div = (float)clock_hz /
        ((float)frequency_hz * (float)(top + 1u));

    if (div < 1.0f || div >= 256.0f)
        return false;

    *wrap = (uint16_t)top;
    *divider = div;

    return true;
}

void mcu_pwm_init(mcu_pwm_t pwm, uint8_t pin, const mcu_pwm_config_t* config)
{
    if (pwm == 0 || !config)
        return;

    if (find_handle(pwm))
        return;

    rp_pwm_handle_t* h = find_free_handle();

    if (!h)
        return;

    uint slice = pwm_gpio_to_slice_num(pin);
    uint channel = pwm_gpio_to_channel(pin);

    rp_pwm_slice_t* s = &slices[slice];

    /*
     * A pin's PWM slice/channel may also be exposed on another
     * GPIO. Never claim a channel already in use.
     */
    if (s->channel_active[channel])
        return;

    uint16_t wrap;
    float divider;

    if (!calculate_pwm(config->frequency_hz, config->resolution_bits, &wrap, &divider))
        return;

    if (s->configured)
    {
        if (s->wrap != wrap || s->resolution_bits != config->resolution_bits || s->frequency_hz != config->frequency_hz)
            return;
    }
    else
    {
        pwm_config hw_config = pwm_get_default_config();

        pwm_config_set_wrap(&hw_config, wrap);
        pwm_config_set_clkdiv(&hw_config, divider);

        /*
         * Start disabled. Configure the output before enabling.
         * Do not call pwm_init() again for the other channel.
         */
        pwm_init(slice, &hw_config, false);

        s->configured = true;
        s->wrap = wrap;
        s->resolution_bits = config->resolution_bits;
        s->frequency_hz = config->frequency_hz;
        s->divider = divider;
    }

    s->channel_active[channel] = true;
    s->channel_inverted[channel] = config->inverted;
    s->channel_pin[channel] = pin;

    pwm_set_output_polarity(slice, s->channel_inverted[PWM_CHAN_A], s->channel_inverted[PWM_CHAN_B]);
    gpio_set_function(pin, GPIO_FUNC_PWM);

    h->active = true;
    h->pwm = pwm;
    h->pin = pin;
    h->slice = (uint8_t)slice;
    h->channel = (uint8_t)channel;
    h->max_duty = (uint16_t)((1u << config->resolution_bits) - 1u);

    pwm_set_chan_level(slice, channel, 0);
    pwm_set_enabled(slice, true);
}

void mcu_pwm_deinit(mcu_pwm_t pwm)
{
    rp_pwm_handle_t* h = find_handle(pwm);

    if (!h)
        return;

    uint slice = h->slice;
    uint channel = h->channel;

    rp_pwm_slice_t* s = &slices[slice];

    pwm_set_chan_level(slice, channel, 0);

    gpio_set_function(h->pin, GPIO_FUNC_SIO);
    gpio_set_dir(h->pin, GPIO_OUT);
    gpio_put(h->pin, false);

    s->channel_active[channel] = false;
    s->channel_inverted[channel] = false;
    s->channel_pin[channel] = 0;

    if (!s->channel_active[PWM_CHAN_A] && !s->channel_active[PWM_CHAN_B])
    {
        pwm_set_enabled(slice, false);
        memset(s, 0, sizeof(*s));
    }
    else
    {
        pwm_set_output_polarity(slice, s->channel_inverted[PWM_CHAN_A], s->channel_inverted[PWM_CHAN_B]);
    }

    memset(h, 0, sizeof(*h));
}

void mcu_pwm_set_duty(mcu_pwm_t pwm, uint16_t duty)
{
    rp_pwm_handle_t* h = find_handle(pwm);

    if (!h || duty > h->max_duty)
        return;

    pwm_set_chan_level(h->slice, h->channel, duty);
}

void mcu_pwm_set_enabled(mcu_pwm_t pwm, bool enabled)
{
    rp_pwm_handle_t* h = find_handle(pwm);

    if (!h)
        return;

    pwm_set_enabled(h->slice, enabled);
}

void mcu_pwm_set_frequency(mcu_pwm_t pwm, uint32_t frequency_hz)
{
    rp_pwm_handle_t* h = find_handle(pwm);

    if (!h)
        return;

    rp_pwm_slice_t *slice = &slices[h->slice];

    uint16_t new_wrap;
    float new_divider;

    if (!calculate_pwm(frequency_hz, slice->resolution_bits, &new_wrap, &new_divider))
        return;

    for (uint8_t channel = 0; channel < NUM_PWM_CHANNELS_PER_SLICE; channel++)
    {
        if (!slice->channel_active[channel])
            continue;

        for (size_t i = 0; i < NUM_PWM_CHANNELS_PER_SLICE * NUM_PWM_SLICES; i++)
        {
            if (!handles[i].active ||
                handles[i].slice != h->slice ||
                handles[i].channel != channel)
            {
                continue;
            }

            if (handles[i].pwm != pwm && (slice->frequency_hz != frequency_hz))
                return;
        }
    }

    pwm_config config = pwm_get_default_config();

    pwm_config_set_wrap(&config, new_wrap);
    pwm_config_set_clkdiv(&config, new_divider);
    pwm_init(h->slice, &config, false);

    slice->wrap = new_wrap;
    slice->divider = new_divider;
    slice->frequency_hz = frequency_hz;
}