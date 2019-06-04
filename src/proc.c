#include "proc.h"
#include "klog.h"
#include "kmalloc.h"
#include "mmap.h"
#include "string.h"
#include "mmu.h"
#include "syscall.h"
#include "interrupts.h"

struct Process *curr_proc = NULL;
struct Process *next_proc = NULL;
struct Process main_proc = {
   .name = "main_proc"
};

static struct ProcessState {
   struct ProcessQueue ready_queue;
   struct LinkedList procs;
} proc_state = {
   .ready_queue = PROC_QUEUE_INIT(proc_state.ready_queue),
   .procs = LINKED_LIST_INIT(proc_state.procs, struct Process, procs),
};

void PROC_queue_init(struct ProcessQueue *queue)
{
   LL_init(&queue->list, offsetof(struct Process, queue));
}

void PROC_queue_enqueue(struct ProcessQueue *queue, struct Process *proc)
{
   LL_enqueue(&queue->list, proc);
}

struct Process *PROC_queue_dequeue(struct ProcessQueue *queue)
{
   return LL_dequeue(&queue->list);
}

int PROC_queue_empty(struct ProcessQueue *queue)
{
   return LL_empty(&queue->list);
}

static void process_exit()
{
   klog(KLOG_LEVEL_INFO, "process %s exited", curr_proc->name);

   LL_del(&curr_proc->procs);

   MMU_free_stack(curr_proc->stack);
   kfree(curr_proc);
   curr_proc = NULL;

   yield();
}

void PROC_reschedule()
{
   IRQ_disable();

   if (curr_proc)
      PROC_queue_enqueue(&proc_state.ready_queue, curr_proc);
   next_proc = PROC_queue_dequeue(&proc_state.ready_queue);

   IRQ_enable();
}

void PROC_yield()
{
   PROC_reschedule();
}

void PROC_exit()
{
   process_exit();
}

void PROC_dump_procs()
{
   struct Process *curr;

   LL_for_each(&proc_state.procs, curr)
      printk("%s\n", curr->name);
   printk("\n");
}

int PROC_create_process(const char *name, kproc_t entry_point, void *arg)
{
   struct Process *new = NULL;

   new = kmalloc(sizeof(struct Process));
   if (!new)
      goto error;
   memset(new, 0, sizeof(struct Process));

   /* Allocate stack */
   new->stack = MMU_alloc_stack();
   if (!new->stack)
      goto error;

   new->stack[0] = ptr_to_int(NULL);
   new->stack[-1] = ptr_to_int(NULL);
   new->stack[-2] = ptr_to_int(&process_exit);
   new->rsp = ptr_to_int(&new->stack[-2]);
   new->rbp = ptr_to_int(&new->stack[-1]);
   new->rip = ptr_to_int(entry_point);
   new->rdi = ptr_to_int(arg);
   new->cs = 0x10;
   new->name = name;

   PROC_queue_enqueue(&proc_state.ready_queue, new);
   LL_enqueue(&proc_state.procs, new);

   klog(KLOG_LEVEL_INFO, "starting process %s", new->name);
   return 0;
error:
   if (new) kfree(new);
   klog(KLOG_LEVEL_WARN, "failed to start process %s", new->name);
   return -1;
}

void PROC_run()
{
   if (PROC_queue_empty(&proc_state.ready_queue))
      return;

   curr_proc = &main_proc;

   yield();
}