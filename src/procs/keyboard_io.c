#include "../klog.h"
#include "../fs.h"
#include "../printk.h"
#include "procs.h"

void keyboard_io(void *arg)
{
   struct OFile *f;
   char buff;

   f = FS_open("ps2", 0);
   if (!f) {
      klog(KLOG_LEVEL_WARN, "failed to initialize keyboard driver");
      return;
   }

   while (1) {
      FS_read(f, &buff, 1);
      printk("%c", buff);
   }
}