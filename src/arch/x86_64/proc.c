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