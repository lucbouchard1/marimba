#include "multiboot.h"
#include "types.h"
#include "printk.h"

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"

int MB_parse_multiboot(uint32_t mb_magic, uint32_t mb_addr)
{
   //size_t size;
   struct MultibootTag *tag;

   if (mb_magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
      printk("error: OS not booted by multiboot complient bootloader\n");
      return -1;
   }

   //size = *(unsigned *) mb_addr;

   for (tag = (struct MultibootTag *) (mb_addr + 8); tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct MultibootTag *) ((uint8_t *) tag + ((tag->size + 7) & ~7))) {
      printk("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
   }

   return 0;
}

#pragma GCC diagnostic pop