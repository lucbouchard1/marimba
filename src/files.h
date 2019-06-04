#ifndef __FILES_H__
#define __FILES_H__

#include "types.h"

#define FILE_TYPE_CHAR_DEVICE 'c'
#define FILE_TYPE_BLOCK_DEVICE 'b'

struct FileData;

struct FileDescriptor {
   struct File *file;
   void *user_Data;
};

struct File {
   struct FileDescriptor *(*open)(struct File *file, uint32_t flags);
   void (*close)(struct FileDescriptor *fd);
   int (*read)(struct FileDescriptor *fd, char *dest, size_t len);

   char type;
   void *dev_data;
   const char *name;

   struct FileData kern_data;
};

void FILE_register(struct File *file);
struct FileDescriptor *FILE_open(const char *name, uint32_t flags);
void FILE_close(struct FileDescriptor *fd);
void FILE_temp_dev_init();

#endif