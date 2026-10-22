#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdbool.h>
#include <stdint.h>

typedef uint8_t mcu_adc_t;

typedef struct
{
    uint8_t pin;
    uint8_t resolution_bits;

} mcu_adc_config_t;

void mcu_adc_init(mcu_adc_t adc, const mcu_adc_config_t* cfg);
void mcu_adc_deinit(mcu_adc_t adc);
uint32_t mcu_adc_read(mcu_adc_t adc);

#ifdef __cplusplus
}
#endif