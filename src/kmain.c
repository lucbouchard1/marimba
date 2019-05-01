#include "vga.h"
#include "printk.h"
#include "io.h"
#include "interrupts.h"
#include "hw_init.h"
#include "drivers/keyboard/keyboard.h"

void keyboard_isr(int irq, int err, void *arg)
{
   struct KeyboardDevice *dev = (struct KeyboardDevice *)arg;
   char c;

   c = dev->read_char(dev);
   printk("%c", c);
}

void kmain(void)
{
   struct KeyboardDevice *kdev;   
   //int counter, i;
   //long delay;

   HW_init();
   VGA_clear();
   IRQ_init();

   kdev = init_ps2(1);
   IRQ_set_handler(0x21, keyboard_isr, kdev);
   IRQ_clear_mask(0x21); // Enable interrupts from keyboard!!
   while(1)
      asm("hlt;");
}