#include <stdint.h>

#include "vga.h"
#include "string.h"
#include "interrupts.h"

#define X86_VGA_ADDR 0xb8000

struct VGATextBuffer {
   unsigned int width;            /* Character width of the text buffer */
   unsigned int height;           /* Character height of the text buffer */
   unsigned int len;              /* Total characters shown by the text buffer */
   unsigned int char_bytes;       /* Bytes allocated in buffer to each char */
   unsigned int x;                /* Horizontal position of cursor */
   unsigned int y;                /* Vertical position of cursor */
   uint16_t *buf;                 /* Memory address of text buffer */
};

static struct VGATextBuffer disp = {
   .width = 80,
   .height = 25,
   .len = 80 * 25,
   .char_bytes = 2,
   .x = 0,
   .y = 0,
   .buf = (void *)X86_VGA_ADDR
};

static void scroll()
{
   memcpy(disp.buf, disp.buf + disp.width,
         (disp.len - disp.width) * disp.char_bytes);
   memset(disp.buf + (disp.len - disp.width), 0, disp.width * disp.char_bytes);
}

void VGA_clear()
{
   memset(disp.buf, 0, disp.len * disp.char_bytes);
   disp.x = disp.y = 0;
}

void VGA_display_char(char c)
{
   IRQ_disable();

   if ((disp.y*disp.width) + disp.x == disp.len) {
      scroll();
      disp.y--;
   }

   if (c == '\n') {
      disp.y += 1 + (disp.x/disp.width);
      disp.x = 0;
   } else if (c == '\r') {
      disp.x = 0;
   } else {
      disp.buf[(disp.y*disp.width) + disp.x] = 0x0f00 | c;
      disp.x++;
   }

   IRQ_enable();
}

void VGA_display_str(const char *str)
{
   const char *cur;

   for (cur = str; *cur; cur++)
      VGA_display_char(*cur);
}