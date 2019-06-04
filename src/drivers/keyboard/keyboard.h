#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define LEFT_SHIFT_SCAN_CODE 0x12
#define RIGHT_SHIFT_SCAN_CODE 0x59
#define RELEASED_SCAN_CODE 0xf0

char translate_scan_code(uint8_t shift, uint8_t code);

#endif