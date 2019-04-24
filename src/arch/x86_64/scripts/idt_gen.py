

target_offset_setup = """\
   template.target_0_15 = (uint16_t)isr_{0};
   template.target_16_31 = (uint16_t)((uint64_t)isr_{0} >> 16);
   template.target_32_63 = (uint32_t)((uint64_t)isr_{0} >> 32);\
"""

print('#include "../../string.h"')
print('#include <stdint.h>')
print('#pragma GCC diagnostic push')
print('#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"')

for irq in range(256):
    print('extern int isr_{0}();'.format(irq))

print("""
struct IDTEntry {
   uint16_t target_0_15;
   uint16_t target_selector;
   uint16_t ist: 3;
   uint16_t rsvd1: 5;
   uint16_t type: 4;
   uint16_t zero: 1;
   uint16_t dpl: 2;
   uint16_t present: 1;
   uint16_t target_16_31;
   uint32_t target_32_63;
   uint32_t rsvd2;
} __attribute__ ((__packed__));

static struct IDTEntry template = {
   .type = 0xE,
   .present = 1,
   .dpl = 0,
   .ist = 0,
   .zero = 0,
   .target_selector = 0x8
};
static struct IDTEntry IDT[256];
static struct {
   uint16_t length;
   void* base;
} __attribute__((packed)) IDT_desc;
""")

print('void IDT_init()')
print('{')

for irq in range(256):
    print(target_offset_setup.format(irq))
    print('   memcpy(&IDT[{0}], &template, sizeof(struct IDTEntry));'.format(irq))
    print('')

print('   IDT_desc.length =  (sizeof(struct IDTEntry)*256) - 1;')
print('   IDT_desc.base = &IDT[0];')
print('   asm ( "lidt %0" : : "m"(IDT_desc) );')
print('}')
print('#pragma GCC diagnostic pop')
