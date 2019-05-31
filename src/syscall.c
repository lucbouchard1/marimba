#include "syscall.h"
#include "printk.h"
#include "interrupts.h"

void syscall_handler(uint32_t num)
{
   printk("System Call: %d\n", num);
}