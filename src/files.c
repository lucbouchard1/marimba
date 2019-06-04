#include "list.h"
#include "files.h"
#include "string.h"
#include "klog.h"

struct FileData {
   struct ListHeader files;
};

static struct Files {
   struct LinkedList list;
} files = {
   .list = LINKED_LIST_INIT(files.list, struct File, kern_data.files)
};

void FILE_register(struct File *file)
{
   LL_enqueue(&files.list, file);
}

struct FileDescriptor *FILE_open(const char *name, uint32_t flags)
{
   struct File *curr;
   struct FileDescriptor *ret = NULL;

   LL_for_each(&files.list, curr) {
      if (!strcmp(name, curr->name)) {
         ret = curr->open(curr, flags);
         break;
      }
   }

   if (!ret)
      klog(KLOG_LEVEL_DEBUG, "failed to open file %s", name);
   return ret;
}

void FILE_close(struct FileDescriptor *fd)
{
   fd->file->close(fd);
}

void FILE_dump_files()
{
   struct File *curr;

   LL_for_each(&files.list, curr)
      printk("%s\n", curr->name);
}

/**
 * Temporary solution for registering available devices
 */
extern struct File ps2_file;
void FILE_temp_dev_init()
{
   struct File *curr;

   FILE_register(&ps2_file);
   klog(KLOG_LEVEL_INFO, "ps2 device driver registered");
}