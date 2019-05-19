#include "elf.h"
#include "printk.h"
#include "utils.h"

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

int ELF_parse_section_headers(struct PhysicalMMap *mmap, void *start,
      unsigned int num_headers, unsigned int str_table_index)
{
   struct ELFSectionHeader *headers = (struct ELFSectionHeader *)start;
   const char *str_table;
   int i;

   if (num_headers > MAX_KERNEL_SECTIONS) {
      printk("error: max kernel sections exceeded in elf file\n");
      return -1;
   }

   str_table = (const char *)headers[str_table_index].section_addr;

   mmap->num_kernel_sects = 0;
   for (i = 0; i < num_headers; i++) {
      if (headers[i].section_type == 0) /* entry unused */
         continue;
      mmap->kernel_sects[mmap->num_kernel_sects].base = headers[i].section_addr;
      mmap->kernel_sects[mmap->num_kernel_sects].length = headers[i].section_size;
      mmap->kernel_sects[mmap->num_kernel_sects].section_name = &str_table[headers[i].sym_str_table_idx];
      mmap->num_kernel_sects++;
   }
   sort_kernel_section_array(mmap->kernel_sects, mmap->num_kernel_sects);

   return 0;
}