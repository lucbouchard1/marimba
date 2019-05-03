#include "vga.h"
#include "printk.h"
#include "io.h"
#include "interrupts.h"
#include "hw_init.h"
#include "multiboot.h"
#include "drivers/keyboard/keyboard.h"
#include "drivers/serial/serial.h"

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
   //int counter, i;
   //long delay;

   VGA_clear();

   MB_parse_multiboot(mb_magic, mb_addr);

   HW_init();

   kdev = init_ps2(1);
   IRQ_set_handler(0x21, keyboard_isr, kdev);
   IRQ_clear_mask(0x21); // Enable interrupts from keyboard!!
   while(1)
      asm("hlt;");
}