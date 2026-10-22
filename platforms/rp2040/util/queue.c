#include "platform/util/queue.h"
#include "pico/critical_section.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct mcu_queue
{
    critical_section_t lock;

    uint8_t* data;
    size_t item_size;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    bool initialized;

} rp_queue_t;

bool mcu_queue_init(mcu_queue_t** queue, size_t item_size, size_t capacity)
{
    if (item_size == 0 || capacity == 0 || !queue)
        return false;

    if (capacity > SIZE_MAX / item_size)
        return false;

    mcu_queue_t* queue_ptr = (mcu_queue_t*)malloc(sizeof(rp_queue_t));
    if (!queue_ptr)
        return false;

    *queue = queue_ptr;

    queue_ptr->data = (uint8_t*)calloc(capacity, item_size);
    queue_ptr->item_size = item_size;
    queue_ptr->capacity = capacity;
    queue_ptr->head = 0;
    queue_ptr->tail = 0;
    queue_ptr->count = 0;

    critical_section_init(&queue_ptr->lock);
    queue_ptr->initialized = true;

    return queue_ptr->data != NULL;
}

void mcu_queue_deinit(mcu_queue_t* queue)
{
    critical_section_deinit(&queue->lock);
    free(queue->data);
    free(queue);
}

bool mcu_queue_try_push(mcu_queue_t* queue, const void* item)
{
    if (!queue || !item || !queue->initialized)
        return false;

    critical_section_enter_blocking(&queue->lock);
    
    if (queue->count == queue->capacity)
    {
        critical_section_exit(&queue->lock);
        return false;
    }

    memcpy(queue->data + queue->tail * queue->item_size, item, queue->item_size);
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;

    critical_section_exit(&queue->lock);
    return true;
}

bool mcu_queue_try_pop(mcu_queue_t* queue, void* item)
{
    if (!queue || !item || !queue->initialized)
        return false;

    critical_section_enter_blocking(&queue->lock);

    if (queue->count == 0)
    {
        critical_section_exit(&queue->lock);
        return false;
    }


    memcpy(item, queue->data + queue->head * queue->item_size, queue->item_size);
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;

    critical_section_exit(&queue->lock);
    return true;
}

size_t mcu_queue_count(mcu_queue_t* queue)
{
    if (!queue || !queue->initialized)
        return 0;

    critical_section_enter_blocking(&queue->lock);
    size_t c = queue->count;
    critical_section_exit(&queue->lock);

    return c;
}

bool mcu_queue_is_empty(mcu_queue_t* queue)
{
    return mcu_queue_count(queue) == 0;
}

bool mcu_queue_is_full(mcu_queue_t* queue)
{
    if (!queue || !queue->initialized)
        return false;

    critical_section_enter_blocking(&queue->lock);
    bool full = queue->count == queue->capacity;
    critical_section_exit(&queue->lock);

    return full;
}

void mcu_queue_reset(mcu_queue_t* queue)
{
    if (!queue || !queue->initialized)
        return;

    critical_section_enter_blocking(&queue->lock);
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    critical_section_exit(&queue->lock);
}