#include "platform/util/queue.h"
#include <stdlib.h>
#include <string.h>

bool mcu_queue_init(mcu_queue_t* queue, size_t item_size, size_t capacity)
{
    if (item_size == 0 || capacity == 0 || !queue)
        return false;

    if (capacity > SIZE_MAX / item_size)
        return false;

    queue->data = (uint8_t*)calloc(capacity, item_size);
    queue->item_size = item_size;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;

    queue->initialized = true;

    return queue->data != NULL;
}

void mcu_queue_deinit(mcu_queue_t* queue)
{
    free(queue->data);
    free(queue);
}

bool mcu_queue_try_push(mcu_queue_t* queue, const void* item)
{
    if (!queue || !item || !queue->initialized)
        return false;
    
    if (queue->count == queue->capacity)
    {
        return false;
    }

    memcpy(queue->data + queue->tail * queue->item_size, item, queue->item_size);
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;

    return true;
}

bool mcu_queue_try_pop(mcu_queue_t* queue, void* item)
{
    if (!queue || !item || !queue->initialized)
        return false;

    if (queue->count == 0)
    {
        return false;
    }

    memcpy(item, queue->data + queue->head * queue->item_size, queue->item_size);
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    return true;
}

size_t mcu_queue_count(mcu_queue_t* queue)
{
    if (!queue || !queue->initialized)
        return 0;

    return queue->count;
}

bool mcu_queue_is_empty(mcu_queue_t* queue)
{
    return mcu_queue_count(queue) == 0;
}

bool mcu_queue_is_full(mcu_queue_t* queue)
{
    if (!queue || !queue->initialized)
        return false;

    return queue->count == queue->capacity;
}

void mcu_queue_reset(mcu_queue_t* queue)
{
    if (!queue || !queue->initialized)
        return;

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}