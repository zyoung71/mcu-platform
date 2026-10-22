#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t time_t;
typedef int32_t timer_id_t;
typedef void (*timer_callback_t)(void* context);

#define INVALID_TIMER_ID -1

time_t mcu_get_time_since_boot_us(void);

timer_id_t mcu_add_timer_periodic(time_t period_us, timer_callback_t cb, void* context);
timer_id_t mcu_add_timer_once(time_t period_us, timer_callback_t cb, void* context);

bool mcu_timer_cancel(timer_id_t id);

#ifdef __cplusplus
}
#endif