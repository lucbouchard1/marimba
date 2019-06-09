#include "../klog.h"
#include "../files.h"
#include "../printk.h"
#include "procs.h"

void keyboard_io(void *arg)
{
   struct OpenFile *f;
   char buff;

   f = FILE_open("ps2", 0);
   if (!f) {
      klog(KLOG_LEVEL_WARN, "failed to initialize keyboard driver");
      return;
   }

   while (1) {
      FILE_read(f, &buff, 1);
      printk("%c", buff);
   }
}