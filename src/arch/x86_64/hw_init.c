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

struct SegmentDescriptor {
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

struct TSSDescriptor {
   struct SegmentDescriptor seg;
   uint32_t base_32_63;
   uint8_t rsvd2;
   uint8_t zero: 5;
   uint8_t rsvd3: 3;
   uint16_t rsvd4;
} __attribute__((__packed__));

union GDTEntry {
   struct SegmentDescriptor seg;
   struct TSSDescriptor tss;
} __attribute__((__packed__));

static struct TSS tss;
static union GDTEntry GDT[3];

static struct {
   uint16_t length;
   void *base;
} __attribute__((packed)) GDTR;

void init_gdt()
{
   GDTR.length = (sizeof(union GDTEntry)*3) - 1;
   GDTR.base = GDT;
   uint8_t tss_desc_offset;

   memset(&tss, 0, sizeof(struct TSS));
   tss.ist[0] = &stack1_top;
   tss.ist[1] = &stack2_top;
   tss.ist[2] = &stack3_top;
   tss.ist[3] = &stack4_top;

   memset(GDT, 0, sizeof(union GDTEntry)*3);
   GDT[1].seg.is_code_segment = 1;
   GDT[1].seg.one = 1;
   GDT[1].seg.present = 1;
   GDT[1].seg.l = 1; /* Main code entry */

   GDT[2].tss.seg.is_code_segment = 1;
   GDT[2].tss.seg.one = 0;
   GDT[2].tss.seg.present = 1;
   GDT[2].tss.seg.l = 1;
   GDT[2].tss.seg.a = 1;
   GDT[2].tss.zero = 0; /* TSS entry */


   GDT[2].tss.seg.limit_0_15 = sizeof(struct TSS);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
   GDT[2].tss.seg.base_0_15 = (uint16_t)&tss;
   GDT[2].tss.seg.base_16_23 = (uint8_t)((uint64_t)&tss >> 16);
   GDT[2].tss.seg.base_24_31 = (uint8_t)((uint64_t)&tss >> 24);
   GDT[2].tss.base_32_63 = (uint32_t)((uint64_t)&tss >> 32);
#pragma GCC diagnostic pop

   tss_desc_offset = 2*sizeof(union GDTEntry);

   asm( "lgdt %0" : : "m"(GDTR) );
   asm( "ltr %w0" : : "r"(tss_desc_offset) );
}

void HW_init()
{
   init_gdt();
}