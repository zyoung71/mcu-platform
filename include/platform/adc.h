#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdbool.h>
#include <stdint.h>

bool mcu_adc_init(uint8_t adc_pin);
bool mcu_adc_deinit(uint8_t adc_pin);
uint32_t mcu_adc_read(uint8_t adc_pin);

#ifdef __cplusplus
}
#endif