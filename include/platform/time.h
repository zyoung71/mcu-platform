#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t mcu_time_t;
typedef int32_t mcu_timer_id_t;
typedef void (*mcu_timer_callback_t)(void* context);

#define INVALID_TIMER_ID -1

mcu_time_t mcu_get_time_since_boot_us(void);

mcu_timer_id_t mcu_add_timer_periodic(mcu_time_t period_us, mcu_timer_callback_t cb, void* context);
mcu_timer_id_t mcu_add_timer_once(mcu_time_t period_us, mcu_timer_callback_t cb, void* context);

bool mcu_timer_cancel(mcu_timer_id_t id);

#ifdef __cplusplus
}
#endif