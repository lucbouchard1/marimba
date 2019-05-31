#include "syscall.h"
#include "klog.h"
#include "interrupts.h"
#include "proc.h"

void syscall_handler(uint32_t num)
{
   switch (num) {
      case 0:
         PROC_yield();
         break;
      
      default:
         klog(KLOG_LEVEL_WARN, "received unsupported system call %d", num);
         break;
   }
}