#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "types.h"

void syscall_handler(uint32_t num);

#if ARCH == x86_64
#include "arch/x86_64/syscall.h"
#else
#error
#endif

#endif