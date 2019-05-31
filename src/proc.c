#include "proc.h"
#include "printk.h"
#include "kmalloc.h"
#include "mmap.h"
#include "string.h"
#include "mmu.h"
#include "syscall.h"

struct Process *curr_proc = NULL;
struct Process *next_proc = NULL;

static struct ProcessState {
   struct Process *ready;
   struct Process *ready_tail;
} proc;

static void thread_entry(void *arg)
{
   ((struct Process *)arg)->entry(((struct Process *)arg)->arg);
}

void PROC_reschedule()
{
   if (curr_proc) {
      proc.ready_tail->next = curr_proc;
      proc.ready_tail = curr_proc;
      curr_proc->next = NULL;
   }

   next_proc = proc.ready;
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

   new->arg = arg;
   new->entry = entry_point;
   new->rsp = ptr_to_int(new->stack);
   new->rip = ptr_to_int(&thread_entry);
   new->cs = 0x8;

   new->next = proc.ready;
   proc.ready = new;
   if (!proc.ready_tail)
      proc.ready_tail = new;

   return 0;
}

void PROC_run()
{
   if (proc.ready)
      yield();
}