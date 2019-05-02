#ifndef __IO_H__
#define __IO_H__

#include "drivers/serial/serial.h"

#if ARCH == x86_64
#include "arch/x86_64/io.h"
#else
#error
#endif

extern struct SerialDevice *main_serial_dev;

#endif