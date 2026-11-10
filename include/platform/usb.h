#pragma once

#if 0

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

bool mcu_usb_serial_init(void);
bool mcu_usb_serial_deinit(void);

bool mcu_usb_serial_connected(void);
void mcu_usb_serial_flush(void);
void mcu_usb_serial_task(void);

size_t mcu_usb_serial_available(void);
size_t mcu_usb_serial_read(uint8_t* data, size_t len);
size_t mcu_usb_serial_write(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif
#endif