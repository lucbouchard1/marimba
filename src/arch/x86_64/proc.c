#include "../../proc.h"

void PROC_save_context(struct Process *proc, uint64_t *stack)
{
   proc->rbp = stack[0];
   proc->rax = stack[1];
   proc->r9 = stack[2];
   proc->r8 = stack[3];
   proc->rcx = stack[4];
   proc->rdx = stack[5];
   proc->r11 = stack[6];
   proc->r10 = stack[7];
   proc->rsi = stack[8];
   proc->rdi = stack[9];
   proc->rip = stack[10];
   proc->cs = stack[11] & 0xFFFF;
   proc->rflags = stack[12];
   proc->rsp = stack[13];
   proc->ss = stack[14] & 0xFFFF;

   asm volatile ("mov %%ds, %0" : "=m"(proc->ds));
   asm volatile ("mov %%es, %0" : "=m"(proc->es));
   asm volatile ("mov %%fs, %0" : "=m"(proc->fs));
   asm volatile ("mov %%gs, %0" : "=m"(proc->gs));

   asm volatile ("mov %%r12, %0" : "=m"(proc->r12));
   asm volatile ("mov %%r13, %0" : "=m"(proc->r13));
   asm volatile ("mov %%r14, %0" : "=m"(proc->r14));
   asm volatile ("mov %%r15, %0" : "=m"(proc->r15));

   asm volatile ("mov %%rbx, %0" : "=m"(proc->rbx));
}

void PROC_load_context(struct Process *proc, uint64_t *stack)
{
   stack[0] = proc->rbp;
   stack[1] = proc->rax;
   stack[2] = proc->r9;
   stack[3] = proc->r8;
   stack[4] = proc->rcx;
   stack[5] = proc->rdx;
   stack[6] = proc->r11;
   stack[7] = proc->r10;
   stack[8] = proc->rsi;
   stack[9] = proc->rdi;
   stack[10] = proc->rip;
   stack[11] |= proc->cs;
   stack[12] = proc->rflags;
   stack[13] = proc->rsp;
   stack[14] |= proc->ss;

   asm volatile ("mov %0, %%ds" : : "m"(proc->ds));
   asm volatile ("mov %0, %%es" : : "m"(proc->es));
   asm volatile ("mov %0, %%fs" : : "m"(proc->fs));
   asm volatile ("mov %0, %%gs" : : "m"(proc->gs));

   asm volatile ("mov %0, %%r12" : : "m"(proc->r12));
   asm volatile ("mov %0, %%r13" : : "m"(proc->r13));
   asm volatile ("mov %0, %%r14" : : "m"(proc->r14));
   asm volatile ("mov %0, %%r15" : : "m"(proc->r15));
   asm volatile ("mov %0, %%rbx" : : "m"(proc->rbx));
}