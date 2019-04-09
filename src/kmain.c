#include "vga.h"

void kmain(void)
{
   int counter, i;
   long delay;

   VGA_clear();

   for (counter = 1; counter < 31; counter++) {
      for (i = 0; i < counter; i++) {
         VGA_display_str("a");
         delay = 5000000;
         while (delay--);
      }
      VGA_display_char('\n');
   }

   VGA_clear();
   VGA_display_str("\n\n\n\n\n\n\n");

   for (counter = 1; counter < 95; counter++) {
      VGA_display_char('\r');
      for (i = 0; i < counter; i++)
         VGA_display_str("#");
      delay = 40000000;
      while (delay--);
   }
   VGA_display_str("Hello World\n");
   VGA_display_str("!*&@&E()QWDJLASDOKJSKDHFWKL");

   while(1);
}