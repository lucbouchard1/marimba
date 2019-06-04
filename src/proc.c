#include "proc.h"
#include "klog.h"
#include "kmalloc.h"
#include "mmap.h"
#include "string.h"
#include "mmu.h"
#include "syscall.h"
#include "interrupts.h"

struct Process main_proc = {
   .name = "main_proc"
};
struct Process *curr_proc = &main_proc;
struct Process *next_proc = &main_proc;

static struct ProcessState {
   struct ProcessQueue ready_queue;
   struct LinkedList procs;
   int sleep_curr;
} proc_state = {
   .ready_queue = PROC_QUEUE_INIT(proc_state.ready_queue),
   .procs = LINKED_LIST_INIT(proc_state.procs, struct Process, procs),
};

static void proc_queue_enqueue(struct ProcessQueue *queue, struct Process *proc)
{
   LL_enqueue(&queue->list, proc);
}

static struct Process *proc_queue_dequeue(struct ProcessQueue *queue)
{
   return LL_dequeue(&queue->list);
}

void PROC_queue_init(struct ProcessQueue *queue)
{
   LL_init(&queue->list, offsetof(struct Process, queue));
}

int PROC_queue_empty(struct ProcessQueue *queue)
{
   return LL_empty(&queue->list);
}

/**
 * Block the current process on the passed queue.
 * @queue: pointer to the queue to block on.
 */
void PROC_block_on(struct ProcessQueue *queue, int enable_ints)
{
   proc_queue_enqueue(queue, curr_proc);
   proc_state.sleep_curr = 1;

   if (enable_ints)
      IRQ_enable();

   yield();
}

void PROC_unblock_head(struct ProcessQueue *queue)
{
   struct Process *proc;

   if (PROC_queue_empty(queue))
      return;

   IRQ_disable();

   proc = proc_queue_dequeue(queue);
   proc_queue_enqueue(&proc_state.ready_queue, proc);

   IRQ_enable();
}

void PROC_unblock_all(struct ProcessQueue *queue)
{
   struct Process *proc;

   IRQ_disable();

   while ((proc = proc_queue_dequeue(queue)))
      proc_queue_enqueue(&proc_state.ready_queue, proc);

   IRQ_enable();
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

   if (curr_proc && !proc_state.sleep_curr)
      proc_queue_enqueue(&proc_state.ready_queue, curr_proc);

   if (PROC_queue_empty(&proc_state.ready_queue)) {
      next_proc = &main_proc;
   } else {
      next_proc = proc_queue_dequeue(&proc_state.ready_queue);
      proc_state.sleep_curr = 0;
   }

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

   proc_queue_enqueue(&proc_state.ready_queue, new);
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
   proc_state.sleep_curr = 1;
   yield();
}