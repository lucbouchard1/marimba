#include "vga.h"
#include "printk.h"

void kmain(void)
{
   //int counter, i;
   //long delay;

   VGA_clear();

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

   printk("Integer %d %%\n", -128);
   printk("Char %c %c %c %c\n", 'H', 'i', 'J', 'K');
   printk("String %s %u\n", "Hello World", 1000);
   printk("Hex 0x%x\n", 0x12345abc);
   printk("Shorts %ld %hd %hd", x, 13, 14);

   while(1);
}