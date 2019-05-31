#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#if ARCH == x86_64
#include "arch/x86_64/syscall.h"
#else
#error
#endif

#endif