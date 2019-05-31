#ifndef __PROC_H__
#define __PROC_H__

#include "types.h"

typedef void (*kproc_t)(void*);

struct Process {
   struct Process *next;
   uint64_t rax, rbx, rcx, rdx, rdi, rsi;
   uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
   uint64_t cs, ss, ds, es, fs, gs;
   uint64_t rbp, rsp, rip, rflags;
   kproc_t entry;
   void *stack;
   void *arg;
};

extern struct Process *curr_proc;
extern struct Process *next_proc;

void PROC_yield();
int PROC_create_kthread(kproc_t entry_point, void *arg);

#endif