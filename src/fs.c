#include "list.h"
#include "fs.h"
#include "string.h"
#include "klog.h"
#include "kmalloc.h"

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
   uint16_t sectors_per_fat;
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

struct FATDirEnt {
   char name[11];
   uint8_t attr;
   uint8_t nt;
   uint8_t ct_tenths;
   uint16_t ct;
   uint16_t cd;
   uint16_t ad;
   uint16_t cluster_hi;
   uint16_t mt;
   uint16_t md;
   uint16_t cluster_lo;
   uint32_t size;
} __attribute__((packed));

struct FATLongDirEnt {
   uint8_t order;
   uint16_t first[5];
   uint8_t attr;
   uint8_t type;
   uint8_t checksum;
   uint16_t middle[6];
   uint16_t zero;
   uint16_t last[2];
} __attribute__((packed));

struct PartBlockDev {
   struct BlockDev dev;
   struct BlockDev *phys_dev;
   sect_t sect_start;
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
      klog(KLOG_LEVEL_INFO, "partition %s with fs type 0x%X detected",
            new->dev.name, new->dev.fs_type);
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

int FS_register_fat32(struct PartBlockDev *part)
{
   struct FAT32 fat;
   int sects_per_clust, rsvd_sects, root_clust;
   int num_fats, tot_sects, sects_per_fat;

   if (part->dev.read_block(&part->dev, 0, &fat) < 0) {
      klog(KLOG_LEVEL_WARN, "failed to read from FAT 32 filesystem");
      return -1;
   }

   /* TODO: Use partition label stored in FAT32 */

   if (fat.signature != 0x28 && fat.signature != 0x29) {
      klog(KLOG_LEVEL_WARN, "FAT 32 filesystem on part %s invalid",
         part->dev.name);
      return -1;
   }

   rsvd_sects = fat.bpb.reserved_sectors;
   num_fats = fat.bpb.num_fats;
   sects_per_clust = fat.bpb.sectors_per_cluster;
   tot_sects = fat.bpb.tot_sectors ? fat.bpb.tot_sectors : fat.sectors_per_fat;
   sects_per_fat = fat.bpb.sectors_per_fat ? fat.bpb.sectors_per_fat : fat.sectors_per_fat;
   root_clust = fat.root_cluster_number;

   return 0;
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