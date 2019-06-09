#ifndef __X86_64_IO_H__
#define __X86_64_IO_H__

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val)
{
   asm volatile ( "outb %0, %1"
      : : "a"(val), "Nd"(port)
   );
}

static inline uint8_t inb(uint16_t port)
{
   uint8_t ret;
   asm volatile ( "inb %1, %0"
      : "=a"(ret)
      : "Nd"(port)
   );
   return ret;
}

static inline void outl(uint16_t port, uint32_t val)
{
   asm volatile ( "out %0, %1"
      : : "a"(val), "d"(port)
   );
}

static inline uint32_t inl(uint16_t port)
{
   uint32_t ret;
   asm volatile ( "in %1, %0"
      : "=a"(ret)
      : "d"(port)
   );
   return ret;
}

#endif