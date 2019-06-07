#ifndef __FILES_H__
#define __FILES_H__

#include "types.h"

#define FILE_TYPE_CHAR_DEVICE 'c'
#define FILE_TYPE_BLOCK_DEVICE 'b'

struct File;

struct FileOps {
   struct OpenFile *(*open)(struct File *file, uint32_t flags);
   void (*close)(struct OpenFile *fd);
   int (*read)(struct OpenFile *fd, char *dest, size_t len);
};

struct CharDevice {
   struct FileOps *fops;
   int num_open;
   struct ListHeader cdev_list;
};

struct BlockDevice {
   int num_open;
   struct ListHeader bdev_list;
};

struct OpenFile {
   struct File *file;
};

struct File {
   struct FileOps fops;

   char type;
   void *dev_data;
   const char *name;

   struct ListHeader file_list;
};


void FILE_cdev_init(struct CharDevice *cdev);
int FILE_register_chrdev(struct CharDevice *cdev, const char *name);
struct OpenFile *FILE_open(const char *name, uint32_t flags);
void FILE_read(struct OpenFile *fd, char *buff, size_t len);
void FILE_close(struct OpenFile *fd);
void FILE_temp_dev_init();

#endif