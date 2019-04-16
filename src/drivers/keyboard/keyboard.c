#include <stdint.h>

#include "keyboard.h"

char translate_scan_code(struct KeyboardDevice *dev, uint8_t code)
{
   switch (code)
   {
      case 0x1c:
         return dev->shift_pressed ? 'A' : 'a';
      case 0x32:
         return dev->shift_pressed ? 'B' : 'b';
      case 0x21:
         return dev->shift_pressed ? 'C' : 'c';
      case 0x23:
         return dev->shift_pressed ? 'D' : 'd';
      case 0x24:
         return dev->shift_pressed ? 'E' : 'e';
      case 0x2b:
         return dev->shift_pressed ? 'F' : 'f';
      case 0x34:
         return dev->shift_pressed ? 'G' : 'g';
      case 0x33:
         return dev->shift_pressed ? 'H' : 'h';
      case 0x43:
         return dev->shift_pressed ? 'I' : 'i';
      case 0x3b:
         return dev->shift_pressed ? 'J' : 'j';
      case 0x42:
         return dev->shift_pressed ? 'K' : 'k';
      case 0x4b:
         return dev->shift_pressed ? 'L' : 'l';
      case 0x3a:
         return dev->shift_pressed ? 'M' : 'm';
      case 0x31:
         return dev->shift_pressed ? 'N' : 'n';
      case 0x44:
         return dev->shift_pressed ? 'O' : 'o';
      case 0x4d:
         return dev->shift_pressed ? 'P' : 'p';
      case 0x15:
         return dev->shift_pressed ? 'Q' : 'q';
      case 0x2d:
         return dev->shift_pressed ? 'R' : 'r';
      case 0x1b:
         return dev->shift_pressed ? 'S' : 's';
      case 0x2c:
         return dev->shift_pressed ? 'T' : 't';
      case 0x3c:
         return dev->shift_pressed ? 'U' : 'u';
      case 0x2a:
         return dev->shift_pressed ? 'V' : 'v';
      case 0x1d:
         return dev->shift_pressed ? 'W' : 'w';
      case 0x22:
         return dev->shift_pressed ? 'X' : 'x';
      case 0x35:
         return dev->shift_pressed ? 'Y' : 'y';
      case 0x1a:
         return dev->shift_pressed ? 'Z' : 'z';
      case 0x5A:
         return '\n';
      case 0x29:
         return ' ';
      default:
         return '?';
   }
}