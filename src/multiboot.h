#ifndef __MULTIBOOT_H__
#define __MULTIBOOT_H__

#include <stdint.h>

#include "types.h"

int MB_parse_multiboot(struct PhysicalMMap *dest, uint32_t mb_magic, uint32_t mb_addr);


#endif