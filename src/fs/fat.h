#ifndef __FAT_H__
#define __FAT_H__

#include "../fs.h"

int FS_register_fat32(struct PartBlockDev *part);

#endif