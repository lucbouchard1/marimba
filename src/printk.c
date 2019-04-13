#include <stdarg.h>
#include <stdint.h>

#include "printk.h"
#include "vga.h"

void print_uint64(uint64_t d)
{
   char buf[64], *cur = buf+64;
   *cur = 0;
   cur--;

   for (; d; d /= 10, cur--)
      *cur = (d % 10) + '0';
   VGA_display_str(cur + 1);
}

void print_int64(int64_t d)
{
   if (d < 0) {
      VGA_display_char('-');
      d *= -1;
   }

   print_uint64(d);
}

int printk(const char *fmt, ...)
{
   int i_arg;
   unsigned int ui_arg;
   const char *cur, *str_arg;
   va_list args;
   va_start(args, fmt);

   for (cur = fmt; *cur; cur++) {
      if (*cur == '%') {
         switch (*(cur + 1))
         {
            case 's':
               str_arg = va_arg(args, const char *);
               VGA_display_str(str_arg);
               break;

            case 'c':
               i_arg = va_arg(args, int);
               VGA_display_char(i_arg);
               break;

            case 'd':
               i_arg = va_arg(args, int);
               print_int64(i_arg);
               break;

            case 'u':
               ui_arg = va_arg(args, int);
               print_uint64(ui_arg);
               break;
         
            default:
               break;
         }
         cur++;
      } else {
         VGA_display_char(*cur);
      }
   }

   return 0;
}