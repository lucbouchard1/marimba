#include "elf.h"
#include "printk.h"

struct ELFSectionHeader {
   uint32_t sym_str_table_idx;
   uint32_t section_type;
   offset_t section_flags;
   void *section_addr;
   offset_t section_offset;
   size_t section_size;
   uint32_t section_link;
   uint32_t section_info;
   offset_t section_addralign;
   size_t section_entsize;
} __attribute__((packed));

int ELF_parse_section_headers(struct KernelELFInfo *elf, void *start,
      unsigned int num_headers, unsigned int str_table_index)
{
   struct ELFSectionHeader *headers = (struct ELFSectionHeader *)start;
   const char *str_table;
   int i;

   str_table = (const char *)headers[str_table_index].section_addr;

   for (i = 0; i < num_headers; i++)
      printk("%s\n", &str_table[headers[i].sym_str_table_idx]);

   return 0;
}