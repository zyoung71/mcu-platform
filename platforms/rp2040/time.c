#include "platform/time.h"
#include "pico/time.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#if PICO_RP2040
#define MAX_TIMERS 4
#elif PICO_RP2350
#define MAX_TIMERS 8
#else
#error "Unknown pico-sdk related hardware."
#endif

static_assert(sizeof(mcu_time_t) == sizeof(absolute_time_t), "Time integer types must be equal.");

typedef struct timer_slot
{
    bool active;
    bool periodic;

    mcu_timer_id_t id;
    mcu_timer_callback_t cb;
    void* context;

    repeating_timer_t repeat;
    alarm_id_t alarm;

} rp_timer_slot_t;

rp_timer_slot_t timer_slots[MAX_TIMERS] = {0};
mcu_timer_id_t next_id = 0;

static void reset_timer_slot(rp_timer_slot_t* slot)
{
    memset(slot, 0, sizeof(*slot));
    slot->id = INVALID_TIMER_ID;
    slot->alarm = -1;
}

static rp_timer_slot_t* find_free_slot(void)
{
    for (uint i = 0; i < MAX_TIMERS; i++)
    {
        if (!timer_slots[i].active)
            return &timer_slots[i];
    }
    return NULL;
}

static rp_timer_slot_t* find_timer(mcu_timer_id_t id)
{
    for (uint i = 0; i < MAX_TIMERS; ++i) {
        if (timer_slots[i].active && timer_slots[i].id == id)
            return &timer_slots[i];
    }
    return NULL;
}

mcu_time_t mcu_get_time_since_boot_us(void)
{
    return to_us_since_boot(get_absolute_time());
}

static bool repeating_timer_callback_intermediate(repeating_timer_t* rtimer)
{
    rp_timer_slot_t* timer = (rp_timer_slot_t*)rtimer->user_data;

    timer->cb(timer->context);

    return true;
}

mcu_timer_id_t mcu_add_timer_periodic(mcu_time_t period_us, mcu_timer_callback_t cb, void* context)
{
    rp_timer_slot_t* slot = find_free_slot();
    if (slot)
    {
        slot->active = true;
        slot->periodic = true;
        slot->id = next_id++;
        slot->cb = cb;
        slot->context = context;

        memset(&slot->repeat, 0, sizeof(slot->repeat));
        slot->alarm = -1;
    }
    else
    {
        return INVALID_TIMER_ID;
    }

    if (add_repeating_timer_us(period_us, &repeating_timer_callback_intermediate, context, &slot->repeat))
        return slot->id;
    else
    {
        reset_timer_slot(slot);
        return INVALID_TIMER_ID;
    }
}

static int64_t alarm_timer_callback_intermediate(alarm_id_t id, void* context)
{
    rp_timer_slot_t* timer = (rp_timer_slot_t*)context;
    
    timer->cb(timer->context);

    return 0;
}

mcu_timer_id_t mcu_add_timer_once(mcu_time_t period_us, mcu_timer_callback_t cb, void* context)
{
    rp_timer_slot_t* slot = find_free_slot();
    if (slot)
    {
        slot->active = true;
        slot->periodic = false;
        slot->id = next_id++;
        slot->cb = cb;
        slot->context = context;

        memset(&slot->repeat, 0, sizeof(slot->repeat));
    }
    else
    {
        return INVALID_TIMER_ID;
    }

    alarm_id_t alarm_id = add_alarm_in_us(period_us, &alarm_timer_callback_intermediate, context, true);
    if (alarm_id >= 0)
    {
        slot->alarm = alarm_id;
        return slot->id;
    }   
    else
    {
        reset_timer_slot(slot);
        return INVALID_TIMER_ID;
    }
}

bool mcu_timer_cancel(mcu_timer_id_t id)
{
    rp_timer_slot_t* timer = find_timer(id);
    bool ret = cancel_repeating_timer(&timer->repeat) || cancel_alarm(timer->alarm);
    reset_timer_slot(timer);
    return ret;
}