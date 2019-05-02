#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "../../types.h"

struct SerialDevice {
   int (*write)(struct SerialDevice *dev, char c);
};

struct SerialDevice *init_x86_serial();

int SER_write_char(struct SerialDevice *dev, char c);
int SER_write_str(struct SerialDevice *dev, const char *str);
int SER_write(struct SerialDevice *dev, const char *buff, size_t len);

#endif