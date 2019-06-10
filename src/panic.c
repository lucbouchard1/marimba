#include "panic.h"
#include "klog.h"

void panic()
{
   klog(KLOG_LEVEL_EMERG, "KERNEL PANIC");
   asm("hlt");
}