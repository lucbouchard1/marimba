#include "boot.h"
#include "printk.h"
#include "elf.h"
#include "utils.h"
#include "klog.h"

#define MULTIBOOT2_BOOTLOADER_MAGIC 0x36d76289

#define MULTIBOOT_TAG_ALIGN 8
#define MULTIBOOT_TAG_TYPE_END 0
#define MULTIBOOT_TAG_TYPE_CMDLINE 1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME 2
#define MULTIBOOT_TAG_TYPE_MODULE 3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO 4
#define MULTIBOOT_TAG_TYPE_BOOTDEV 5
#define MULTIBOOT_TAG_TYPE_MMAP 6
#define MULTIBOOT_TAG_TYPE_VBE 7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER 8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS 9
#define MULTIBOOT_TAG_TYPE_APM 10

#define MULTIBOOT_MEMORY_AVAILABLE 1
#define MULTIBOOT_MEMORY_RESERVED 2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE 3
#define MULTIBOOT_MEMORY_NVS 4


struct MultibootTag {
   uint32_t type;
   uint32_t size;
} __attribute__((packed));

struct MultibootMMapEntry {
   uint64_t addr;
   uint64_t len;
   uint32_t type;
   uint32_t zero;
} __attribute__((packed));

struct MultibootMMapTag {
   uint32_t type;
   uint32_t size;
   uint32_t entry_size;
   uint32_t entry_version;
   struct MultibootMMapEntry entries[1];
} __attribute__((packed));

struct MultibootELFTag {
   uint32_t type;
   uint32_t size;
   uint32_t num;
   uint32_t entsize;
   uint32_t shndx;
   char sections[1];
} __attribute__((packed));

static int parse_mmap(struct MultibootMMapTag *mmap, struct PhysicalMMap *dest)
{
   struct MultibootMMapEntry *curr = mmap->entries;
   int entries = mmap->size/mmap->entry_size, i;

   dest->num_mmap = 0;
   for (i = 0; i < entries; i++, curr++) {
      if (curr->type != MULTIBOOT_MEMORY_AVAILABLE)
         continue;

      if (dest->num_mmap == MAX_MMAP_ENTRIES) {
         klog(KLOG_LEVEL_EMERG, "max mmap entries exceeded in multiboot info");
         return -1;
      }

      dest->ram_sects[dest->num_mmap].base = (void *)curr->addr;
      dest->ram_sects[dest->num_mmap].length = curr->len;
      dest->num_mmap++;
   }
   sort_mmap_entry_array(dest->ram_sects, dest->num_mmap);

   dest->total_ram = (size_t)dest->ram_sects[dest->num_mmap-1].base +
         dest->ram_sects[dest->num_mmap-1].length;

   return 0;
}

static int parse_elf(struct MultibootELFTag *elf, struct PhysicalMMap *dest)
{
   return ELF_parse_section_headers(dest, elf->sections, elf->num, elf->shndx);
}

int BOOT_parse_multiboot(struct PhysicalMMap *dest, uint32_t mb_magic, uint32_t mb_addr)
{
   struct MultibootTag *tag = int_to_ptr(mb_addr + 8);

   if (mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
      klog(KLOG_LEVEL_EMERG, "OS not booted by multiboot compliant bootloader");
      return -1;
   }

   for (; tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct MultibootTag *) ((uint8_t *) tag + ((tag->size + 7) & ~7))) {
      switch (tag->type) {
         case MULTIBOOT_TAG_TYPE_MMAP:
            if (parse_mmap((struct MultibootMMapTag *)tag, dest) < 0)
               return -1;
            break;

         case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
            if (parse_elf((struct MultibootELFTag *)tag, dest) < 0)
               return -1;
            break;

         default:
            break;
      }
   }

   return 0;
}