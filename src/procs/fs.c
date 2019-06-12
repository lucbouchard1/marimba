#include "../fs.h"
#include "../syscall.h"
#include "../klog.h"
#include "../kmalloc.h"
#include "procs.h"

struct FAT_BPB {
   uint8_t jmp[3];
   char oem_id[8];
   uint16_t bytes_per_sector;
   uint8_t sectors_per_cluster;
   uint16_t reserved_sectors;
   uint8_t num_fats;
   uint16_t num_dirents;
   uint16_t tot_sectors;
   uint8_t mdt;
   uint16_t num_sectors_per_fat;
   uint16_t sectors_per_track;
   uint16_t num_heads;
   uint32_t num_hidden_sectors;
   uint32_t large_sector_count;
} __attribute__((packed));

struct FAT32 {
   struct FAT_BPB bpb;
   uint32_t sectors_per_fat;
   uint16_t flags;
   uint8_t major_vers;
   uint8_t minor_vers;
   uint32_t root_cluster_number;
   uint16_t fsinfo_sector;
   uint16_t backup_boot_sector;
   uint8_t zero[12];
   uint8_t drive_num;
   uint8_t nt_flags;
   uint8_t signature;
   uint32_t serial_num;
   char label[11];
   char sys_id[8];
   uint8_t boot_code[420];
   uint8_t boot_sig[2];
} __attribute__((packed));

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

void dump_fat(struct BlockDev *blk, sect_t block)
{
   struct FAT32 fat;
   //int i;

   // buff = kmalloc(blk->blk_size);
   // if (!buff) {
   //    klog(KLOG_LEVEL_WARN, "could not allocate memory");
   //    return;
   // }

   blk->read_block(blk, block, &fat);

   printk("Signature: %x\n", fat.signature);
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

   dump_fat(part, 0);

   while (1)
      yield();
}