#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__

#include <stdint.h>

void IRQ_init();
void IRQ_set_mask(unsigned char irq);
void IRQ_clear_mask(unsigned char irq);
void IRQ_end_of_interrupt(unsigned char irq);
void IRQ_enable();
void IRQ_disable();

typedef void (*irq_handler_t)(int irq, int err, void* arg);
void IRQ_set_handler(int irq, irq_handler_t handler, void *arg);

#if ARCH == x86_64
#include "arch/x86_64/interrupts.h"
#else
#error
#endif

#endif