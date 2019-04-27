#include <stdint.h>

#include "../../hw_init.h"

struct GDTEntry {
   uint16_t limit_0_15;
   uint16_t base_0_15;
   uint8_t base_16_23;

   uint8_t a: 1;
   uint8_t w: 1;
   uint8_t e: 1;
   uint8_t is_code_segment: 1;
   uint8_t one: 1;
   uint8_t dpl: 2;

   uint8_t present: 1;
   uint8_t segment_limit_16_19: 4;
   uint8_t avl: 1;
   uint8_t l: 1;
   uint8_t d: 1;
   uint8_t g: 1;

   uint8_t base_24_31;
} __attribute__ ((__packed__));

static struct GDTEntry GDT[] = {
   {0}, /* Zero entry */
   {.is_code_segment = 1, .one = 1, .present = 1, .l = 1}
};

static struct {
   uint16_t length;
   void *base;
} __attribute__((packed)) GDTR;

void init_gdt()
{
   GDTR.length = (sizeof(struct GDTEntry)*2) - 1;
   GDTR.base = GDT;

   asm( "lgdt %0" : : "m"(GDTR) );
}

void HW_init()
{
   init_gdt();
}