#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

typedef struct mcu_queue mcu_queue_t;

bool mcu_queue_init(mcu_queue_t** queue, size_t item_size, size_t capacity);
void mcu_queue_deinit(mcu_queue_t* queue);

bool mcu_queue_try_push(mcu_queue_t* queue, const void* item);
bool mcu_queue_try_pop(mcu_queue_t* queue, void* item);

size_t mcu_queue_count(mcu_queue_t* queue);
bool mcu_queue_is_empty(mcu_queue_t* queue);
bool mcu_queue_is_full(mcu_queue_t* queue);
void mcu_queue_reset(mcu_queue_t* queue);

#ifdef __cplusplus
}
#endif