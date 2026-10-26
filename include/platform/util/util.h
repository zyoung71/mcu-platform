#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "platform/platform_config.h"

#define INVALID_UNUSED_GPIO ((uint8_t)-1)
#define is_gpio_invalid_or_unused(pin) (pin == INVALID_UNUSED_GPIO || pin >= MCU_NUM_GPIO)

#ifdef __cplusplus
}
#endif