#include "proc.h"
#include "printk.h"
#include "kmalloc.h"
#include "mmap.h"
#include "string.h"
#include "mmu.h"
#include "syscall.h"
#include "interrupts.h"

struct Process *curr_proc = NULL;
struct Process *next_proc = NULL;
struct Process main_proc;

static struct ProcessState {
   struct Process *ready;
   struct Process *ready_tail;
} proc_state;

static void thread_exit()
{
   printk("%p exited\n", curr_proc);

   MMU_free_stack(curr_proc->stack);

   if (curr_proc->prev)
      curr_proc->prev->next = curr_proc->next;
   else
      proc_state.ready = curr_proc->next;

   kfree(curr_proc);
   curr_proc = NULL;
   yield();
}

void PROC_reschedule()
{
   IRQ_disable();

   if (curr_proc) {
      proc_state.ready_tail->next = curr_proc;
      curr_proc->prev = proc_state.ready_tail;
      proc_state.ready_tail = curr_proc;
      curr_proc->next = NULL;
   }

   next_proc = proc_state.ready;
   proc_state.ready->next->prev = NULL;
   proc_state.ready = proc_state.ready->next;

   IRQ_enable();
}

void PROC_yield()
{
   //printk("PROC YIELD!\n");
   PROC_reschedule();
}

int PROC_create_kthread(kproc_t entry_point, void *arg)
{
   struct Process *new;

   new = kmalloc(sizeof(struct Process));
   if (!new)
      return -1;
   memset(new, 0, sizeof(struct Process));

   /* Allocate stack */
   new->stack = MMU_alloc_stack();
   if (!new->stack) {
      kfree(new);
      return -1;
   }

   new->stack[0] = ptr_to_int(NULL);
   new->stack[-1] = ptr_to_int(NULL);
   new->stack[-2] = ptr_to_int(&thread_exit);
   new->rsp = ptr_to_int(&new->stack[-2]);
   new->rbp = ptr_to_int(&new->stack[-1]);

   new->rip = ptr_to_int(entry_point);
   new->rdi = ptr_to_int(arg);
   new->cs = 0x10;
   new->entry = entry_point;
   new->arg = arg;

   if (proc_state.ready)
      proc_state.ready->prev = new;
   new->next = proc_state.ready;
   new->prev = NULL;
   proc_state.ready = new;
   if (!proc_state.ready_tail)
      proc_state.ready_tail = new;

   return 0;
}

void PROC_run()
{
   if (!proc_state.ready)
      return;

   curr_proc = &main_proc;
   main_proc.next = NULL;

   yield();
}