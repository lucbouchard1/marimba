#include <stdarg.h>
#include <stdint.h>

#include "printk.h"
#include "vga.h"

void print_uint64(uint64_t d)
{
   char buf[64], *cur = buf+64;
   *cur = 0;
   cur--;

   if (!d) {
      VGA_display_char('0');
      return;
   }

   for (; d; d /= 10, cur--)
      *cur = (d % 10) + '0';
   VGA_display_str(cur + 1);
}

void print_uint64_hex(uint64_t d)
{
   char buf[64], *cur = buf+64;
   unsigned int digit;
   *cur = 0;
   cur--;

   if (!d) {
      VGA_display_char('0');
      return;
   }

   for (; d; d /= 16, cur--) {
      digit = (d % 16);
      if (digit < 10)
         *cur = digit + '0';
      else
         *cur = (digit - 10) + 'a';
   }
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
   void *ptr_arg;
   long l_arg;
   int64_t q_arg;
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
               ui_arg = va_arg(args, unsigned int);
               print_uint64(ui_arg);
               break;

            case 'x':
               ui_arg = va_arg(args, unsigned int);
               print_uint64_hex(ui_arg);
               break;

            case 'l':
               l_arg = va_arg(args, long);
               switch (*(cur + 2))
               {
                  case 'u':
                     print_uint64(l_arg);
                     break;
                  case 'd':
                     print_int64(l_arg);
                     break;
                  case 'x':
                     print_uint64_hex(l_arg);
                     break;
                  default:
                     break;
               }
               cur++;
               break;

            case 'h':
               i_arg = va_arg(args, int);
               switch (*(cur + 2))
               {
                  case 'u':
                     print_uint64(i_arg);
                     break;
                  case 'd':
                     print_int64(i_arg);
                     break;
                  case 'x':
                     print_uint64_hex(i_arg);
                     break;
                  default:
                     break;
               }
               cur++;
               break;

            case 'q':
               q_arg = va_arg(args, int64_t);
               switch (*(cur + 2))
               {
                  case 'u':
                     print_uint64(q_arg);
                     break;
                  case 'd':
                     print_int64(q_arg);
                     break;
                  case 'x':
                     print_uint64_hex(q_arg);
                     break;
                  default:
                     break;
               }
               cur++;
               break;

            case 'p':
               ptr_arg = va_arg(args, void *);
               print_uint64_hex((uint64_t)ptr_arg);
               break;

            case '%':
               VGA_display_char('%');               
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