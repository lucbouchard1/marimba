#ifndef __PROC_H__
#define __PROC_H__

#include "types.h"

struct Process {
   uint64_t rax, rbx, rcx, rdx, rdi, rsi;
   uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
   uint64_t cs, ss, ds, es, fs, gs;
   uint64_t rbp, rsp, rip, rflags;
};

void PROC_yield();

#endif