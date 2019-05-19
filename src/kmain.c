#include "vga.h"
#include "printk.h"
#include "io.h"
#include "interrupts.h"
#include "hw_init.h"
#include "mmu.h"
#include "multiboot.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/serial.h"

static struct SystemMMap map;

void keyboard_isr(int irq, int err, void *arg)
{
   struct KeyboardDevice *dev = (struct KeyboardDevice *)arg;
   char c;

   c = dev->read_char(dev);
   printk("%c", c);
}

void kmain(uint32_t mb_magic, uint32_t mb_addr)
{
   struct KeyboardDevice *kdev;
   int i;
   //long delay;

   VGA_clear();
   HW_init();

   MB_parse_multiboot(&map, mb_magic, mb_addr);

   printk("Total Memory %ld\n", map.total_ram);

   printk("Ram MMAP:\n");
   for (i = 0; i < map.num_mmap; i++)
      printk("Addr: %p   Len: %lx\n",
            map.ram_sects[i].base, map.ram_sects[i].length);

   printk("\nKernel Sections:\n");
   for (i = 0; i < map.num_kernel_sects; i++)
      printk("Addr: %p   Len: 0x%lx   Name: %s\n", map.kernel_sects[i].base,
            map.kernel_sects[i].length, map.kernel_sects[i].section_name);

   MMU_init(&map);
   MMU_stress_test();

   kdev = init_ps2(1);
   IRQ_set_handler(0x21, keyboard_isr, kdev);
   IRQ_clear_mask(0x21); // Enable interrupts from keyboard!!
   while(1)
      asm("hlt;");
}