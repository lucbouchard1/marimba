#include <stdint.h>
#include "../../string.h"

#include "../../hw_init.h"

#define GDT_LENGTH 3

#define GDT_USER_CODE_TYPE_EXECUTE_ONLY 0x8
#define GDT_SYS_TYPE_TSS 0x9

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

   uint8_t limit_16_19: 4;
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
static struct GDTEntry GDT[GDT_LENGTH];
static struct {
   uint16_t length;
   void *base;
} __attribute__((packed)) GDTR;

void gdt_user_init(struct GDTEntry *gdt, uint32_t base, uint32_t limit, uint8_t type)
{
   memset(gdt, 0, sizeof(struct GDTEntry));

   gdt->user_segment = 1;
   gdt->type = type;
   gdt->present = 1;
   gdt->l = 1;

   gdt->limit_0_15 = (uint16_t)limit;
   gdt->limit_16_19 = 0xF & limit >> 16;

   gdt->base_0_15 = (uint16_t)base;
   gdt->base_16_23 = (uint8_t)(base >> 16);
   gdt->base_24_31 = (uint8_t)(base >> 24);
}

void gdt_sys_init(struct GDTEntry *gdt, uint64_t base, uint32_t limit, uint8_t type)
{
   gdt_user_init(gdt, base, limit, type);
   gdt->user_segment = 0;
   GDT[2].sys.tss.base_32_63 = (uint32_t)(base >> 32);
}

void init_gdt()
{
   GDTR.length = (sizeof(struct GDTEntry)*GDT_LENGTH) - 1;
   GDTR.base = GDT;

   /* Clear out GDT and TSS */
   memset(GDT, 0, sizeof(struct GDTEntry)*GDT_LENGTH);
   memset(&tss, 0, sizeof(struct TSS));

   /* Setup TSS */
   tss.ist[0] = &stack1_top;
   tss.ist[1] = &stack2_top;
   tss.ist[2] = &stack3_top;
   tss.ist[3] = &stack4_top;

   /* Setup code GDT entry */
   gdt_user_init(&GDT[1], 0, 0, GDT_USER_CODE_TYPE_EXECUTE_ONLY);

   /* Setup TSS GDT entry */
   gdt_sys_init(&GDT[2], (uint64_t)&tss, sizeof(struct TSS), GDT_SYS_TYPE_TSS);

   asm( "lgdt %0" : : "m"(GDTR) );
   asm( "ltr %w0" : : "r"(2*sizeof(struct GDTEntry)) );
}

void HW_init()
{
   init_gdt();
}