#include "../fs.h"
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

   klog(KLOG_LEVEL_WARN, "doing second read");
   blk->read_block(blk, block, buff);
   klog(KLOG_LEVEL_WARN, "done doing second read");

   for (i = 0; i < blk->blk_size; i++) {
      printk("%x ", buff[i]);
      if (i && !(i % 32))
         printk("\n");
   }
}

void filesystem_init(void *arg)
{
   struct BlockDev *blk, *part;
   struct MasterBootRecord mbr;

   blk = BLK_open("ata");
   if (!blk) {
      klog(KLOG_LEVEL_WARN, "failed to create block device");
      return;
   }

   blk->read_block(blk, 0, &mbr);
   FS_process_mbr(blk, &mbr);

   part = BLK_open("ata_1");
   if (!part) {
      klog(KLOG_LEVEL_WARN, "failed to create block device");
      return;
   }

   FS_register_fs(part);

   while (1)
      yield();
}