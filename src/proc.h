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
void PROC_dump_procs();
void PROC_queue_init(struct ProcessQueue *queue);
int PROC_queue_empty(struct ProcessQueue *queue);
void PROC_unblock_all(struct ProcessQueue *queue);
void PROC_block_on(struct ProcessQueue *queue, int enable_ints);
void PROC_unblock_head(struct ProcessQueue *queue);
void PROC_queue_dump(struct ProcessQueue *queue);
void PROC_ready_dump();

#if ARCH == x86_64
#include "arch/x86_64/proc.h"
#else
#error
#endif

#endif