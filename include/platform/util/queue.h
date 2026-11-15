#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

typedef struct mcu_protected_queue mcu_protected_queue_t;

typedef struct
{
    uint8_t* data;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    bool initialized;
} mcu_queue_t;

bool mcu_protected_queue_init(mcu_protected_queue_t** queue, size_t item_size, size_t capacity);
bool mcu_queue_init(mcu_queue_t* queue, size_t item_size, size_t capacity);

void mcu_protected_queue_deinit(mcu_protected_queue_t* queue);
void mcu_queue_deinit(mcu_queue_t* queue);

bool mcu_protected_queue_try_push(mcu_protected_queue_t* queue, const void* item);
bool mcu_queue_try_push(mcu_queue_t* queue, const void* item);

bool mcu_protected_queue_try_pop(mcu_protected_queue_t* queue, void* item);
bool mcu_queue_try_pop(mcu_queue_t* queue, void* item);

size_t mcu_protected_queue_count(mcu_protected_queue_t* queue);
size_t mcu_queue_count(mcu_queue_t* queue);

bool mcu_protected_queue_is_empty(mcu_protected_queue_t* queue);
bool mcu_queue_is_empty(mcu_queue_t* queue);

bool mcu_protected_queue_is_full(mcu_protected_queue_t* queue);
bool mcu_queue_is_full(mcu_queue_t* queue);

void mcu_protected_queue_reset(mcu_protected_queue_t* queue);
void mcu_queue_reset(mcu_queue_t* queue);

#ifdef __cplusplus
}
#endif