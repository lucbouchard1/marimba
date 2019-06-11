#ifndef __FILES_H__
#define __FILES_H__

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
   const char *name;
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

void FILE_process_mbr(struct BlockDev *dev, struct MasterBootRecord *mbr);
void FILE_cdev_init(struct CharDev *cdev, struct FileOps *fops);
int FILE_register_cdev(struct CharDev *cdev, const char *name);
int BLK_register(struct BlockDev *bdev);
struct BlockDev *BLK_open(const char *name);
struct OFile *FILE_open(const char *name, uint32_t flags);
void FILE_read(struct OFile *file, char *buff, size_t len);
void FILE_close(struct OFile *file);
void FILE_temp_dev_init();

#endif