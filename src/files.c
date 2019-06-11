#include "list.h"
#include "files.h"
#include "string.h"
#include "klog.h"
#include "kmalloc.h"

static struct Files {
   struct LinkedList inodes;
   struct LinkedList blocks;
   struct LinkedList chars;
} files = {
   .inodes = LINKED_LIST_INIT(files.inodes, struct INode, inode_list),
   .blocks = LINKED_LIST_INIT(files.blocks, struct BlockDev, bdev_list),
   .chars = LINKED_LIST_INIT(files.blocks, struct CharDev, cdev_list)
};

void FILE_process_mbr(struct BlockDev *dev, struct MasterBootRecord *mbr)
{
   int i;

   printk("Boot Signature: %x\n", mbr->boot_sig);

   for (i = 0; i < 4; i++) {
      printk("Partition %d:\n", i);
      printk("Start LBA: %x\n", mbr->parts[i].start_lba);
      printk("Num Sectors: %x\n", mbr->parts[i].num_sectors);
   }
}

void FILE_cdev_init(struct CharDev *cdev, struct FileOps *fops)
{
   memset(cdev, 0, sizeof(struct CharDev));
   cdev->fops = fops;
}

int FILE_register_cdev(struct CharDev *cdev, const char *name)
{
   struct INode *new;

   new = kmalloc(sizeof(struct INode));
   if (!new)
      return -1;
   new->fops = cdev->fops;
   new->cdev = cdev;
   new->name = name;
   new->type = FILE_TYPE_CHAR_DEVICE;
   cdev->inode = new;

   LL_enqueue(&files.inodes, new);
   LL_enqueue(&files.chars, cdev);
   return 0;
}

int BLK_register(struct BlockDev *bdev)
{
   LL_enqueue(&files.blocks, bdev);
   return 0;
}

struct BlockDev *BLK_open(const char *name)
{
   struct BlockDev *curr;

   LL_for_each(&files.blocks, curr)
      if (!strcmp(name, curr->name))
         return curr;

   klog(KLOG_LEVEL_DEBUG, "failed to open block device %s", name);
   return NULL;
}

struct OFile *FILE_open(const char *name, uint32_t flags)
{
   struct INode *curr;
   struct OFile *new = NULL;
   int ret = -1;

   LL_for_each(&files.inodes, curr) {
      if (!strcmp(name, curr->name)) {
         new = kmalloc(sizeof(struct INode));
         if (!new)
            break;
         new->inode = curr;
         new->fops = curr->fops;
         ret = curr->fops->open(curr, new);
         break;
      }
   }

   if (ret < 0)
      klog(KLOG_LEVEL_DEBUG, "failed to open file %s", name);
   return new;
}

void FILE_read(struct OFile *file, char *buff, size_t len)
{
   file->fops->read(file, buff, len);
}

void FILE_close(struct OFile *file)
{
   file->fops->close(file);
}

void FILE_dump_files()
{
   struct INode *curr;

   LL_for_each(&files.inodes, curr)
      printk("%s\n", curr->name);
}

/**
 * Temporary solution for registering available devices
 */
extern int ps2_init_module();
extern int ata_init_module();
void FILE_temp_dev_init()
{
   ps2_init_module();
   klog(KLOG_LEVEL_INFO, "ps2 device driver registered");

   ata_init_module();
   klog(KLOG_LEVEL_INFO, "ata device driver registered");
}