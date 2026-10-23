#include "platform/pwm.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include <string.h>
#include <math.h>

#if PICO_RP2040
#define NUM_PWM_CHANNELS_PER_SLICE 2
#elif PICO_RP2350
#define NUM_PWM_CHANNELS_PER_SLICE 2
#endif

typedef struct
{
    bool active;
    bool enabled;
    bool inverted;
    uint8_t pin;
    float duty;

} rp_pwm_channel_t;

typedef struct
{
    rp_pwm_channel_t channels[NUM_PWM_CHANNELS_PER_SLICE];

    float actual_frequency_hz;
    uint16_t divider;
    uint16_t wrap;
    bool configured;

} rp_pwm_slice_t;

static rp_pwm_slice_t slices[NUM_PWM_SLICES] = {0};

static bool calculate_timing(float frequency_hz, uint16_t* wrap, uint16_t* divider, float* actual_frequency_hz)
{
    if (frequency_hz == 0 || !wrap || !divider || !actual_frequency_hz)
        return false;

    const double sysclk = (double)clock_get_hz(clk_sys);

    for (int bits = 16; bits >= 2; --bits) 
    {
        const uint32_t counts = (1u << bits) - 1u;
        const double ideal_divider = (sysclk * 16.0) / ((double)frequency_hz * (double)counts);

        const uint64_t rounded_divider =
            (uint64_t)floor(ideal_divider + 0.5);

        if (rounded_divider < 16u)
            continue;

        if (rounded_divider > 4095u)
            return false;

        const double actual_frequency = (sysclk * 16.0) / ((double)rounded_divider * (double)counts);

        if (!isfinite(actual_frequency) || actual_frequency <= 0.0)
            return false;

        *wrap = (uint16_t)(counts - 1u);
        *divider = (uint16_t)rounded_divider;
        *actual_frequency_hz = (float)actual_frequency;

        return true;
    }

    return false;
}

static bool get_channel(uint8_t pin, rp_pwm_slice_t** slice_state, uint* slice_number, uint* channel_number)
{
    const uint slice = pwm_gpio_to_slice_num(pin);
    const uint channel = pwm_gpio_to_channel(pin);

    if (slice >= NUM_PWM_SLICES)
        return false;

    rp_pwm_slice_t* state = &slices[slice];

    if (!state->configured || !state->channels[channel].active)
        return false;

    if (state->channels[channel].pin != pin)
        return false;

    if (slice_state)
        *slice_state = state;

    if (slice_number)
        *slice_number = slice;

    if (channel_number)
        *channel_number = channel;

    return true;
}

static void update_polarity(uint slice, const rp_pwm_slice_t* state)
{
    pwm_set_output_polarity(
        slice,
        state->channels[PWM_CHAN_A].active &&
            state->channels[PWM_CHAN_A].inverted,
        state->channels[PWM_CHAN_B].active &&
            state->channels[PWM_CHAN_B].inverted
    );
}

static bool any_channel_active(const rp_pwm_slice_t* state)
{
    return state->channels[PWM_CHAN_A].active || state->channels[PWM_CHAN_B].active;
}

bool mcu_pwm_init(uint8_t pin, float frequency_hz, bool inverted)
{
    if (frequency_hz == 0.f)
        return false;

    if (frequency_hz < 0.f)
    {
        frequency_hz = -frequency_hz;
        inverted = !inverted;
    }

    uint16_t wrap;
    uint16_t divider;
    float actual_frequency_hz;

    if (!calculate_timing(frequency_hz, &wrap, &divider, &actual_frequency_hz))
        return false;

    const uint slice = pwm_gpio_to_slice_num(pin);
    const uint channel = pwm_gpio_to_channel(pin);

    rp_pwm_slice_t* state = &slices[slice];

    // note certain pins share a pwm slice. automatic rejection is present
    if (state->channels[channel].active)
        return false;

    if (state->configured)
    {
        if (state->wrap != wrap || state->divider != divider)
            return false;
    }
    else
    {
        pwm_config hardware_config = pwm_get_default_config();

        pwm_config_set_wrap(&hardware_config, wrap);
        pwm_config_set_clkdiv_int_frac4(&hardware_config, (uint8_t)(divider / 16u), (uint8_t)(divider % 16u));
        pwm_init(slice, &hardware_config, true);

        state->configured = true;
        state->wrap = wrap;
        state->divider = divider;
        state->actual_frequency_hz = actual_frequency_hz;
    }

    rp_pwm_channel_t* channel_state = &state->channels[channel];

    channel_state->active = true;
    channel_state->enabled = true;
    channel_state->inverted = inverted;
    channel_state->pin = pin;
    channel_state->duty = 0;

    pwm_set_chan_level(slice, channel, 0);
    update_polarity(slice, state);

    gpio_set_function(pin, GPIO_FUNC_PWM);

    return true;
}

bool mcu_pwm_deinit(uint8_t pin)
{
    rp_pwm_slice_t* state;
    uint slice;
    uint channel;

    if (!get_channel(pin, &state, &slice, &channel))
        return false;

    rp_pwm_channel_t* channel_state = &state->channels[channel];

    gpio_set_function(pin, GPIO_FUNC_SIO);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, channel_state->inverted);

    memset(channel_state, 0, sizeof(*channel_state));

    if (!any_channel_active(state))
    {
        pwm_set_enabled(slice, false);
        memset(state, 0, sizeof(*state));
    }
    else
        update_polarity(slice, state);

    return true;
}

static bool convert_duty(float duty, uint16_t wrap, uint16_t *level)
{
    if (!isfinite(duty) || !level)
        return false;

    if (duty > 1.f)
        duty = 1.f;
    if (duty < 0.f)
        duty = 0.f;

    const uint32_t counts = (uint32_t)wrap + 1u;

    uint32_t value = (uint32_t)floor((double)duty * counts + 0.5);

    if (value > counts)
        value = counts;

    *level = (uint16_t)value;
    return true;
}

bool mcu_pwm_set_duty(uint8_t pin, float duty)
{
    if (!isfinite(duty) || duty < 0.0f || duty > 1.0f)
        return false;

    rp_pwm_slice_t* state;
    uint slice;
    uint channel;

    if (!get_channel(pin, &state, &slice, &channel))
        return false;

    uint16_t level;

    if (!convert_duty(duty, state->wrap, &level))
        return false;

    pwm_set_chan_level(slice, channel, level);
    state->channels[channel].duty = duty;

    return true;
}

bool mcu_pwm_set_enabled(uint8_t pin, bool enabled)
{
    rp_pwm_slice_t* state;
    uint slice;
    uint channel;

    if (!get_channel(pin, &state, &slice, &channel))
        return false;

    rp_pwm_channel_t* channel_state = &state->channels[channel];

    if (channel_state->enabled == enabled)
        return true;

    if (enabled) {
        gpio_set_function(pin, GPIO_FUNC_PWM);
        channel_state->enabled = true;
    } else {
        gpio_set_function(pin, GPIO_FUNC_SIO);
        gpio_set_dir(pin, GPIO_OUT);
        gpio_put(pin, channel_state->inverted);

        channel_state->enabled = false;
    }

    return true;
}

bool mcu_pwm_set_frequency(uint8_t pin, float frequency_hz)
{
    rp_pwm_slice_t* state;
    uint slice;
    uint channel;

    if (!get_channel(pin, &state, &slice, &channel))
        return false;

    uint16_t new_wrap;
    uint16_t new_divider;
    float actual_frequency_hz;

    if (!calculate_timing(frequency_hz, &new_wrap, &new_divider, &actual_frequency_hz))
        return false;

    if (new_wrap == state->wrap && new_divider == state->divider)
    {
        state->actual_frequency_hz = actual_frequency_hz;
        return true;
    }

    const uint other_channel = channel ^ 1u;

    if (state->channels[other_channel].active)
        return false;

    uint16_t level;

    if (!convert_duty(state->channels[channel].duty, new_wrap, &level))
        return false;

    const bool was_enabled = (pwm_hw->slice[slice].csr & PWM_CH0_CSR_EN_BITS) != 0;

    pwm_set_enabled(slice, false);

    pwm_set_wrap(slice, new_wrap);
    pwm_set_clkdiv_int_frac4(
        slice,
        (uint8_t)(new_divider / 16u),
        (uint8_t)(new_divider % 16u));

    pwm_set_chan_level(slice, channel, level);

    state->wrap = new_wrap;
    state->divider = new_divider;
    state->actual_frequency_hz = actual_frequency_hz;

    if (was_enabled)
        pwm_set_enabled(slice, true);

    return true;
}


float mcu_pwm_get_frequency(uint8_t pin)
{
    rp_pwm_slice_t* state;

    if (!get_channel(pin, &state, NULL, NULL))
        return 0.f;

    return state->actual_frequency_hz;
}

float mcu_pwm_get_duty(uint8_t pin)
{
    rp_pwm_slice_t* state;
    uint channel;

    if (!get_channel(pin, &state, NULL, &channel))
        return false;

    return state->channels[channel].duty;
}