#include "vga.h"
#include "printk.h"
#include "io.h"
#include "interrupts.h"
#include "drivers/keyboard/keyboard.h"

void kmain(void)
{
   //int counter, i;
   //long delay;

   VGA_clear();
   init_ps2();
   IRQ_init();
   IRQ_clear_mask(0x21); // Enable interrupts from keyboard!!
   while(1);
   
   // for (counter = 1; counter < 31; counter++) {
   //    for (i = 0; i < counter; i++) {
   //       VGA_display_str("a");
   //       delay = 5000000;
   //       while (delay--);
   //    }
   //    VGA_display_char('\n');
   // }

   // VGA_clear();
   // VGA_display_str("\n\n\n\n\n\n\n");

   // for (counter = 1; counter < 95; counter++) {
   //    VGA_display_char('\r');
   //    for (i = 0; i < counter; i++)
   //       VGA_display_str("#");
   //    delay = 40000000;
   //    while (delay--);
   // }
   // VGA_display_str("Hello World\n");
   // VGA_display_str("!*&@&E()QWDJLASDOKJSKDHFWKL");
   long x = -200;
   int p;

   printk("Integer %d %%\n", -128);
   printk("Char %c %c %c %c\n", 'H', 'i', 'J', 'K');
   printk("String %s %x\n", "Hello World", -1000);
   printk("Hex 0x%x\n", 0x12345abc);
   printk("Hex 0x%X\n", 0x12345abc);
   printk("Shorts %ld %hd %hd\n", x, 13, 14);
   printk("Pointer %p", (void *)&p);

   init_ps2();

   while(1);
}