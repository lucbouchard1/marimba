#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

void IRQ_generic_isr(uint32_t irq);
void IRQ_generic_isr_error(uint32_t irq, uint32_t err);
void IRQ_init();

#if ARCH == x86_64
#include "arch/x86_64/interrupts.h"
#else
#error
#endif

#endif