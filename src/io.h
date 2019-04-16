#ifndef __IO_H__
#define __IO_H__

#if ARCH == x86_64
#include "arch/x86_64/io.h"
#else
#error
#endif

#endif