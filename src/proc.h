#ifndef __PROC_H__
#define __PROC_H__

#include "types.h"
#include "list.h"

#define PROC_QUEUE_INIT(name) {.list=LINKED_LIST_INIT(name.list, struct Process, queue)}

typedef void (*kproc_t)(void*);

struct Process {
   struct ListHeader queue;
   struct ListHeader procs;
   uint64_t rax, rbx, rcx, rdx, rdi, rsi;
   uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
   uint64_t cs, ss, ds, es, fs, gs;
   uint64_t rbp, rsp, rip, rflags;
   uint64_t *stack;
   const char *name;
};

struct ProcessQueue {
   struct LinkedList list;
};

extern struct Process *curr_proc;
extern struct Process *next_proc;

void PROC_yield();
int PROC_create_process(const char *name, kproc_t entry_point, void *arg);
void PROC_run();
void PROC_exit();

#if ARCH == x86_64
#include "arch/x86_64/proc.h"
#else
#error
#endif

#endif