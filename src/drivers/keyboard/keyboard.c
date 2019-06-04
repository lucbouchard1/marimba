#include <stdint.h>

#include "keyboard.h"

char translate_scan_code(uint8_t shift, uint8_t code)
{
   switch (code)
   {
      case 0x1c:
         return shift ? 'A' : 'a';
      case 0x32:
         return shift ? 'B' : 'b';
      case 0x21:
         return shift ? 'C' : 'c';
      case 0x23:
         return shift ? 'D' : 'd';
      case 0x24:
         return shift ? 'E' : 'e';
      case 0x2b:
         return shift ? 'F' : 'f';
      case 0x34:
         return shift ? 'G' : 'g';
      case 0x33:
         return shift ? 'H' : 'h';
      case 0x43:
         return shift ? 'I' : 'i';
      case 0x3b:
         return shift ? 'J' : 'j';
      case 0x42:
         return shift ? 'K' : 'k';
      case 0x4b:
         return shift ? 'L' : 'l';
      case 0x3a:
         return shift ? 'M' : 'm';
      case 0x31:
         return shift ? 'N' : 'n';
      case 0x44:
         return shift ? 'O' : 'o';
      case 0x4d:
         return shift ? 'P' : 'p';
      case 0x15:
         return shift ? 'Q' : 'q';
      case 0x2d:
         return shift ? 'R' : 'r';
      case 0x1b:
         return shift ? 'S' : 's';
      case 0x2c:
         return shift ? 'T' : 't';
      case 0x3c:
         return shift ? 'U' : 'u';
      case 0x2a:
         return shift ? 'V' : 'v';
      case 0x1d:
         return shift ? 'W' : 'w';
      case 0x22:
         return shift ? 'X' : 'x';
      case 0x35:
         return shift ? 'Y' : 'y';
      case 0x1a:
         return shift ? 'Z' : 'z';
      case 0x16:
         return shift ? '!' : '1';
      case 0x1e:
         return shift ? '@' : '2';
      case 0x26:
         return shift ? '#' : '3';
      case 0x25:
         return shift ? '$' : '4';
      case 0x2e:
         return shift ? '%' : '5';
      case 0x36:
         return shift ? '^' : '6';
      case 0x3d:
         return shift ? '&' : '7';
      case 0x3e:
         return shift ? '*' : '8';
      case 0x46:
         return shift ? '(' : '9';
      case 0x45:
         return shift ? ')' : '0';
      case 0x4e:
         return shift ? '_' : '-';
      case 0x55:
         return shift ? '+' : '=';
      case 0x5A:
         return '\n';
      case 0x29:
         return ' ';
      default:
         return '?';
   }
}