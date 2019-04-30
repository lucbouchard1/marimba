#include <stdint.h>
#include "../../string.h"

#include "../../hw_init.h"

extern void *stack1_top;
extern void *stack2_top;
extern void *stack3_top;
extern void *stack4_top;

struct TSS {
   uint32_t rsvd1;
   void *rsp[3];
   uint64_t rsvd2;
   void *ist[7];
   uint64_t rsvd3;
   uint16_t rsvd4;
   uint16_t io_map_base;
} __attribute__((__packed__));

struct TSSDescriptor {
   uint32_t base_32_63;
   uint8_t rsvd2;
   uint8_t zero: 5;
   uint8_t rsvd3: 3;
   uint16_t rsvd4;
} __attribute__((__packed__));

struct GDTEntry {
   uint16_t limit_0_15;
   uint16_t base_0_15;
   uint8_t base_16_23;

   uint8_t type: 4;
   uint8_t user_segment: 1;
   uint8_t dpl: 2;
   uint8_t present: 1;

   uint8_t segment_limit_16_19: 4;
   uint8_t avl: 1;
   uint8_t l: 1;
   uint8_t d: 1;
   uint8_t g: 1;

   uint8_t base_24_31;

   union SystemSegment {
      struct TSSDescriptor tss;
   } sys;
} __attribute__((__packed__));

static struct TSS tss;
static struct GDTEntry GDT[3];

static struct {
   uint16_t length;
   void *base;
} __attribute__((packed)) GDTR;

void init_gdt()
{
   GDTR.length = (sizeof(struct GDTEntry)*3) - 1;
   GDTR.base = GDT;

   memset(&tss, 0, sizeof(struct TSS));
   tss.ist[0] = &stack1_top;
   tss.ist[1] = &stack2_top;
   tss.ist[2] = &stack3_top;
   tss.ist[3] = &stack4_top;

   memset(GDT, 0, sizeof(struct GDTEntry)*3);

   /* Setup code GDT entry */
   GDT[1].user_segment = 1;
   GDT[1].type = 0x8;
   GDT[1].present = 1;
   GDT[1].l = 1;

   /* Setup TSS GDT entry */
   GDT[2].user_segment = 0;
   GDT[2].type = 0x9;
   GDT[2].present = 1;
   GDT[2].l = 1;
   GDT[2].sys.tss.zero = 0; /* TSS entry */

   GDT[2].limit_0_15 = sizeof(struct TSS);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
   GDT[2].base_0_15 = (uint16_t)&tss;
   GDT[2].base_16_23 = (uint8_t)((uint64_t)&tss >> 16);
   GDT[2].base_24_31 = (uint8_t)((uint64_t)&tss >> 24);
   GDT[2].sys.tss.base_32_63 = (uint32_t)((uint64_t)&tss >> 32);
#pragma GCC diagnostic pop

   asm( "lgdt %0" : : "m"(GDTR) );
   asm( "ltr %w0" : : "r"(2*sizeof(struct GDTEntry)) );
}

void HW_init()
{
   init_gdt();
}