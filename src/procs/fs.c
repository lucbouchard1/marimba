#include "../files.h"
#include "../syscall.h"
#include "../klog.h"
#include "../kmalloc.h"
#include "procs.h"

void dump_block(struct BlockDev *blk, sect_t block)
{
   uint8_t *buff;
   int i;

   buff = kmalloc(blk->blk_size);
   if (!buff) {
      klog(KLOG_LEVEL_WARN, "could not allocate memory");
      return;
   }

   blk->read_block(blk, block, buff);

   for (i = 0; i < blk->blk_size; i++) {
      printk("%x ", buff[i]);
      if (i && !(i % 32))
         printk("\n");
   }
}

void filesystem_init(void *arg)
{
   struct BlockDev *blk;

   blk = BLK_open("ata");
   if (!blk) {
      klog(KLOG_LEVEL_WARN, "failed to create block device");
      return;
   }

   dump_block(blk, 0);

   while (1)
      yield();
}