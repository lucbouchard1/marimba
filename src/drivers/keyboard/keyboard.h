#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#define LEFT_SHIFT_SCAN_CODE 0x12
#define RIGHT_SHIFT_SCAN_CODE 0x59
#define RELEASED_SCAN_CODE 0xf0

struct KeyboardDevice {
   char (*read_char)(struct KeyboardDevice *dev);
   int shift_pressed;
};

char translate_scan_code(struct KeyboardDevice *dev, uint8_t code);

struct KeyboardDevice *init_ps2();

#endif