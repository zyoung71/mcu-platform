#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

bool mcu_pwm_init(uint8_t pin, float frequency_hz, bool inverted);
bool mcu_pwm_deinit(uint8_t pin);

bool mcu_pwm_set_enabled(uint8_t pin, bool enabled);
bool mcu_pwm_set_duty(uint8_t pin, float duty);
bool mcu_pwm_set_frequency(uint8_t pin, float frequency_hz);

float mcu_pwm_get_duty(uint8_t pin);
float mcu_pwm_get_frequency(uint8_t pin);

#ifdef __cplusplus
}
#endif