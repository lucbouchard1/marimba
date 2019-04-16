#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

struct KeyboardDevice {
   char (*read_char)(struct KeyboardDevice *dev);
};

struct KeyboardDevice *init_ps2();

#endif