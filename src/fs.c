#include "list.h"
#include "fs.h"
#include "string.h"
#include "klog.h"
#include "kmalloc.h"
#include "fs/fat.h"

#define FS_TYPE_FAT32 0x0C

static struct Files {
   struct LinkedList inodes;
   struct LinkedList blocks;
   struct LinkedList chars;
} files = {
   .inodes = LINKED_LIST_INIT(files.inodes, struct INode, inode_list),
   .blocks = LINKED_LIST_INIT(files.blocks, struct BlockDev, bdev_list),
   .chars = LINKED_LIST_INIT(files.blocks, struct CharDev, cdev_list)
};

int part_read_block(struct BlockDev *dev, sect_t blk_num, void *dst)
{
   struct PartBlockDev *part = (struct PartBlockDev *)dev;
   return part->phys_dev->read_block(part->phys_dev,
         blk_num + part->sect_start, dst);
}

int FS_process_mbr(struct BlockDev *dev, struct MasterBootRecord *mbr)
{
   struct PartBlockDev *new;
   int i, name;

   if (mbr->boot_sig != 0xaa55) {
      klog(KLOG_LEVEL_WARN, "%s device does not have valid MBR signature", dev->name);
      return -1;
   }

   for (i = 0; i < 4; i++) {
      if (!mbr->parts[i].start_lba)
         continue;

      new = kmalloc(sizeof(struct PartBlockDev));
      if (!new)
         return -1;
      new->phys_dev = dev;
      new->sect_start = mbr->parts[i].start_lba;
      new->dev.blk_size = dev->blk_size;
      new->dev.total_len = mbr->parts[i].num_sectors * dev->blk_size;
      new->dev.type = PARTITION;  
      new->dev.fs_type = mbr->parts[i].partition_type;
      new->dev.read_block = &part_read_block;
      new->dev.name = kmalloc(20);
      if (!new->dev.name) {
         kfree(new);
         return -1;
      }
      name = strlen(dev->name);
      strcpy(new->dev.name, dev->name);
      new->dev.name[name] = '_';
      new->dev.name[name+1] = '1' + i;
      new->dev.name[name+2] = 0;
      klog(KLOG_LEVEL_INFO, "partition %s with fs type 0x%X at sector 0x%lX with size %ld detected",
            new->dev.name, new->dev.fs_type, new->sect_start, new->dev.total_len);
      BLK_register(&new->dev);
   }

   return 0;
}

void FS_cdev_init(struct CharDev *cdev, struct FileOps *fops)
{
   memset(cdev, 0, sizeof(struct CharDev));
   cdev->fops = fops;
}

int FS_register_cdev(struct CharDev *cdev, const char *name)
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

struct OFile *FS_open(const char *name, uint32_t flags)
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

void FS_read(struct OFile *file, char *buff, size_t len)
{
   file->fops->read(file, buff, len);
}

void FS_close(struct OFile *file)
{
   file->fops->close(file);
}

void FS_dump_files()
{
   struct INode *curr;

   LL_for_each(&files.inodes, curr)
      printk("%s\n", curr->name);
}

int FS_register_fs(struct BlockDev *blk)
{
   struct PartBlockDev *part = (struct PartBlockDev *)blk;

   if (part->dev.fs_type == FS_TYPE_FAT32)
      return FS_register_fat32(part);
   else
   {
      klog(KLOG_LEVEL_WARN, "partition type %x not supported",
            part->dev.fs_type);
      return -1;
   }
}

/**
 * Temporary solution for registering available devices
 */
extern int ps2_init_module();
extern int ata_init_module();
void FS_temp_dev_init()
{
   ps2_init_module();
   klog(KLOG_LEVEL_INFO, "ps2 device driver registered");

   ata_init_module();
   klog(KLOG_LEVEL_INFO, "ata device driver registered");
}