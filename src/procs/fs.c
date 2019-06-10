#include "../files.h"
#include "../syscall.h"
#include "../klog.h"
#include "procs.h"

void filesystem_init(void *arg)
{
   uint16_t buff[256];
   struct BlockDev *blk;

   blk = BLK_open("ata");
   if (!blk) {
      klog(KLOG_LEVEL_WARN, "failed to create block device");
      return;
   }

   blk->read_block(blk, 1, buff);
   printk("Here it is: %x\n", buff[83]);

   while (1)
      yield();
}