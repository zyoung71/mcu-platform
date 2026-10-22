#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t mcu_pwm_t;

typedef struct
{
    uint32_t frequency_hz;
    uint16_t resolution_bits;
    bool inverted;

} mcu_pwm_config_t;

bool mcu_pwm_init(mcu_pwm_t pwm, uint8_t pin, const mcu_pwm_config_t* cfg);
bool mcu_pwm_deinit(mcu_pwm_t pwm);
bool mcu_pwm_set_duty(mcu_pwm_t pwm, uint16_t duty);
bool mcu_pwm_set_frequency(mcu_pwm_t pwm, uint32_t frequency_hz);
bool mcu_pwm_set_enabled(mcu_pwm_t pwm, bool enabled);

#ifdef __cplusplus
}
#endif