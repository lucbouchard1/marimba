#ifndef __ELF_H__
#define __ELF_H__

#include "types.h"

int ELF_parse_section_headers(struct PhysicalMMap *mmap, void *start,
      unsigned int num_headers, unsigned int str_table_index);

#endif