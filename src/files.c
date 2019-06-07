#include "list.h"
#include "files.h"
#include "string.h"
#include "klog.h"
#include "kmalloc.h"

static struct Files {
   struct LinkedList files;
   struct LinkedList blocks;
   struct LinkedList chars;
} files = {
   .files = LINKED_LIST_INIT(files.files, struct File, file_list),
   .blocks = LINKED_LIST_INIT(files.blocks, struct BlockDevice, bdev_list),
   .chars = LINKED_LIST_INIT(files.blocks, struct CharDevice, cdev_list)
};

void FILE_cdev_init(struct CharDevice *cdev, struct FileOps *fops)
{
   memset(cdev, 0, sizeof(struct CharDevice));
   cdev->fops = fops;
}

int FILE_register_chrdev(struct CharDevice *cdev, const char *name)
{
   struct File *new;

   new = kmalloc(sizeof(struct File));
   if (!new)
      return -1;
   memcpy(&new->fops, cdev->fops, sizeof(struct FileOps));
   new->dev_data = cdev;
   new->name = name;
   new->type = FILE_TYPE_CHAR_DEVICE;
   cdev->file = new;

   LL_enqueue(&files.files, new);
   LL_enqueue(&files.chars, cdev);
   return 0;
}

struct OpenFile *FILE_open(const char *name, uint32_t flags)
{
   struct File *curr;
   struct OpenFile *ret = NULL;

   LL_for_each(&files.files, curr) {
      if (!strcmp(name, curr->name)) {
         ret = curr->fops.open(curr, flags);
         break;
      }
   }

   if (!ret)
      klog(KLOG_LEVEL_DEBUG, "failed to open file %s", name);
   return ret;
}

void FILE_read(struct OpenFile *fd, char *buff, size_t len)
{
   fd->file->fops.read(fd, buff, len);
}

void FILE_close(struct OpenFile *fd)
{
   fd->file->fops.close(fd);
}

void FILE_dump_files()
{
   struct File *curr;

   LL_for_each(&files.files, curr)
      printk("%s\n", curr->name);
}

/**
 * Temporary solution for registering available devices
 */
extern int init_ps2();
void FILE_temp_dev_init()
{
   init_ps2();
   klog(KLOG_LEVEL_INFO, "ps2 device driver registered");
}