#ifndef __FILES_H__
#define __FILES_H__

#include "types.h"

#define FILE_TYPE_CHAR_DEVICE 'c'
#define FILE_TYPE_BLOCK_DEVICE 'b'

struct CharDevice {
   int num_open;
};

struct BlockDevice {
   int num_open;
};

struct FileData {
   struct ListHeader files;
};

struct OpenFile {
   struct File *file;
};

struct File {
   struct OpenFile *(*open)(uint32_t flags);
   void (*close)(struct OpenFile *fd);
   int (*read)(struct OpenFile *fd, char *dest, size_t len);

   char type;
   void *dev_data;
   const char *name;

   struct FileData kern_data;
};

void FILE_register(struct File *file);
struct OpenFile *FILE_open(const char *name, uint32_t flags);
void FILE_read(struct OpenFile *fd, char *buff, size_t len);
void FILE_close(struct OpenFile *fd);
void FILE_temp_dev_init();

#endif