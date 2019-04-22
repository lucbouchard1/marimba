#include "../../interrupts.h"
#include "../../printk.h"

void IRQ_generic_isr(uint32_t irq)
{
   printk("Recieved interrupt: %d\n", irq);
}

void IRQ_generic_isr_error(uint32_t irq, uint32_t err)
{
   printk("Recieved error interrupt: %d %d\n", irq, err);
}