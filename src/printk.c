#include <stdarg.h>
#include <stdint.h>

#include "printk.h"
#include "io.h"
#include "vga.h"
#include "drivers/serial/serial.h"

void display_char(char c)
{
   VGA_display_char(c);
   if (main_serial_dev)
      SER_write_char(main_serial_dev, c);
}

void display_str(const char *c)
{
   VGA_display_str(c);
   if (main_serial_dev)
      SER_write_str(main_serial_dev, c);
}

#define HANDLE_FORMAT_SPECIFIER(arg, type, specifier) \
do {                                  \
switch (specifier)                    \
{                                     \
   case 'u':                          \
      print_u##type(arg);             \
      break;                          \
   case 'd':                          \
      print_##type(arg);              \
      break;                          \
   case 'x':                          \
      print_u##type##_hex(arg, 'a');  \
      break;                          \
   case 'X':                          \
      print_u##type##_hex(arg, 'A');  \
      break;                          \
   default:                           \
      break;                          \
}                                     \
} while (0)

#define DISP_UNSIGNED_VAL_DEC(d) do { \
char buf[64], *cur = buf+64;          \
*(cur--) = 0;                         \
if (!d) {                             \
   display_char('0');                 \
   return;                            \
}                                     \
for (; d; d /= 10, cur--)             \
   *cur = (d % 10) + '0';             \
display_str(cur + 1);                 \
} while (0)

#define DISP_UNSIGNED_VAL_HEX(d, bc) do { \
char buf[64], *cur = buf+64;          \
int digit;                            \
*(cur--) = 0;                         \
if (!d) {                             \
   display_char('0');                 \
   return;                            \
}                                     \
for (; d; d /= 16, cur--) {           \
   digit = (d % 16);                  \
   if (digit < 10)                    \
      *cur = digit + '0';             \
   else                               \
      *cur = (digit - 10) + bc ;      \
}                                     \
display_str(cur + 1);                 \
} while (0)

#define DISP_SIGNED_VAL_DEC(d, type) do { \
if (d < 0) {                          \
   display_char('-');                 \
   d *= -1;                           \
}                                     \
print_u##type(d);                     \
} while (0);

void print_uint64_t(uint64_t d){ DISP_UNSIGNED_VAL_DEC(d); }
void print_uint64_t_hex(uint64_t d, char base_char) { DISP_UNSIGNED_VAL_HEX(d, base_char); }
void print_int64_t(int64_t d) { DISP_SIGNED_VAL_DEC(d, int64_t); }

void print_ulong(unsigned long d){ DISP_UNSIGNED_VAL_DEC(d); }
void print_ulong_hex(unsigned long d, char base_char) { DISP_UNSIGNED_VAL_HEX(d, base_char); }
void print_long(long d) { DISP_SIGNED_VAL_DEC(d, long); }

void print_uint(unsigned int d){ DISP_UNSIGNED_VAL_DEC(d); }
void print_uint_hex(unsigned int d, char base_char) { DISP_UNSIGNED_VAL_HEX(d, base_char); }
void print_int(int d) { DISP_SIGNED_VAL_DEC(d, int); }

void print_ushort(unsigned short d){ DISP_UNSIGNED_VAL_DEC(d); }
void print_ushort_hex(unsigned short d, char base_char) { DISP_UNSIGNED_VAL_HEX(d, base_char); }
void print_short(short d) { DISP_SIGNED_VAL_DEC(d, short); }

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
               display_str(str_arg);
               break;

            case 'c':
               i_arg = va_arg(args, int);
               display_char(i_arg);
               break;

            case 'p':
               ptr_arg = va_arg(args, void *);
               display_str("0x");
               print_uint64_t_hex((uint64_t)ptr_arg, 'a');
               break;

            case 'l':
               l_arg = va_arg(args, long);
               HANDLE_FORMAT_SPECIFIER(l_arg, long, *(cur + 2));
               cur++;
               break;

            case 'h':
               i_arg = va_arg(args, int);
               HANDLE_FORMAT_SPECIFIER(i_arg, short, *(cur + 2));
               cur++;
               break;

            case 'q':
               q_arg = va_arg(args, int64_t);
               HANDLE_FORMAT_SPECIFIER(q_arg, int64_t, *(cur + 2));
               cur++;
               break;

            case 'd':
            case 'u':
            case 'x':
            case 'X':
               i_arg = va_arg(args, int);
               HANDLE_FORMAT_SPECIFIER(i_arg, int, *(cur + 1));            
               break;

            case '%':
               display_char('%');               
               break;

            default:
               break;
         }
         cur++;
      } else {
         display_char(*cur);
      }
   }

   return 0;
}