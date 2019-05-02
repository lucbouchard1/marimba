#ifndef __SERIAL_H__
#define __SERIAL_H__

struct SerialDevice {
   int (*write)(struct SerialDevice *dev, char c);
};

#endif