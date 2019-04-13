#include <stdarg.h>
#include <stdint.h>

#include "printk.h"
#include "vga.h"

#define HANDLE_FORMAT_SPECIFIER(arg, type) \
do { \
switch (type) \
{ \
   case 'u': \
      print_uint64(arg); \
      break; \
   case 'd': \
      print_int64(arg); \
      break; \
   case 'x': \
      print_uint64_hex(arg); \
      break; \
   default: \
      break; \
} \
} while (0)

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

            case 'p':
               ptr_arg = va_arg(args, void *);
               print_uint64_hex((uint64_t)ptr_arg);
               break;

            case 'l':
               l_arg = va_arg(args, long);
               HANDLE_FORMAT_SPECIFIER(l_arg, *(cur + 2));
               cur++;
               break;

            case 'h':
               i_arg = va_arg(args, int);
               HANDLE_FORMAT_SPECIFIER(i_arg, *(cur + 2));
               cur++;
               break;

            case 'q':
               q_arg = va_arg(args, int64_t);
               HANDLE_FORMAT_SPECIFIER(q_arg, *(cur + 2));
               cur++;
               break;

            case 'd':
            case 'u':
            case 'x':
               i_arg = va_arg(args, int);
               HANDLE_FORMAT_SPECIFIER(i_arg, *(cur + 1));            
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