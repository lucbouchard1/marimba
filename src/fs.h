#ifndef __FS_H__
#define __FS_H__

#include "types.h"
#include "list.h"

#define FILE_TYPE_CHAR_DEVICE 'c'
#define FILE_TYPE_BLOCK_DEVICE 'b'

struct OFile;
struct INode;

struct FileOps {
   int (*open)(struct INode *inode, struct OFile *file);
   int (*close)(struct OFile *file);
   int (*read)(struct OFile *file, char *dest, size_t len);
};

struct CharDev {
   struct FileOps *fops;
   struct INode *inode;
   struct ListHeader cdev_list;
};

enum BlockDevType { MASS_STORAGE, PARTITION };

struct BlockDev {
   int (*read_block)(struct BlockDev *dev, sect_t blk_num, void *dst);
   size_t blk_size;
   size_t total_len;
   char *name;
   enum BlockDevType type;
   uint8_t fs_type;
   struct ListHeader bdev_list;
};

struct OFile {
   struct INode *inode;
   struct FileOps *fops;
   void *private_data;
};

struct INode {
   struct FileOps *fops;
   unsigned int type;
   union {
      struct CharDev *cdev;
      struct BlockDev *bdev;
   };
   const char *name;
   struct ListHeader inode_list;
};

struct SuperBlock {
   struct INode *root_inode;
};

struct PartitionRecord {
   uint8_t status;
   uint8_t first_chs_h;
   uint8_t first_chs_sh;
   uint8_t first_chs_sl;
   uint8_t partition_type;
   uint8_t last_chs_h;
   uint8_t last_chs_sh;
   uint8_t last_chs_sl;
   uint32_t start_lba;
   uint32_t num_sectors;
} __attribute__((packed));

struct MasterBootRecord {
   uint8_t bootstrap_code[446];
   struct PartitionRecord parts[4];
   uint16_t boot_sig;
} __attribute__((packed));

int FS_process_mbr(struct BlockDev *dev, struct MasterBootRecord *mbr);
void FS_cdev_init(struct CharDev *cdev, struct FileOps *fops);
int FS_register_cdev(struct CharDev *cdev, const char *name);
int BLK_register(struct BlockDev *bdev);
struct BlockDev *BLK_open(const char *name);
struct OFile *FS_open(const char *name, uint32_t flags);
void FS_read(struct OFile *file, char *buff, size_t len);
void FS_close(struct OFile *file);
int FS_register_fs(struct BlockDev *blk);
void FS_temp_dev_init();

#endif