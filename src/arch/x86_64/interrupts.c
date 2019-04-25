#include "../../interrupts.h"
#include "../../printk.h"
#include "../../string.h"
#include "../../io.h"

extern void IDT_init();

#define PIC_MASTER_PORT		0x20		/* IO base address for master PIC */
#define PIC_SLAVE_PORT		0xA0		/* IO base address for slave PIC */
#define PIC_MASTER_COMMAND	PIC_MASTER_PORT
#define PIC_MASTER_DATA	(PIC_MASTER_PORT+1)
#define PIC_SLAVE_COMMAND	PIC_SLAVE_PORT
#define PIC_SLAVE_DATA	(PIC_SLAVE_PORT+1)

#define PIC_MASTER_REMAP_BASE 0x20
#define PIC_SLAVE_REMAP_BASE PIC_MASTER_REMAP_BASE + 8

#define ICW1_ICW4	0x01		/* ICW4 (not) needed */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */
 
#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

void PIC_init()
{
	unsigned char a1, a2;
   int i;

   /* Save masks */
   a1 = inb(PIC_MASTER_DATA);
   a2 = inb(PIC_SLAVE_DATA);
 
   /* Initialize PIC to cascade mode */
   outb(PIC_MASTER_COMMAND, ICW1_INIT | ICW1_ICW4);
   outb(PIC_SLAVE_COMMAND, ICW1_INIT | ICW1_ICW4);

   /* Remap PIC interrupt numbers to remove conflicts */
   outb(PIC_MASTER_DATA, PIC_MASTER_REMAP_BASE);
   outb(PIC_SLAVE_DATA, PIC_SLAVE_REMAP_BASE);
   /* ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100) */
   outb(PIC_MASTER_DATA, 4);
   /* ICW3: tell Slave PIC its cascade identity (0000 0010)*/
   outb(PIC_SLAVE_DATA, 2);

   outb(PIC_MASTER_DATA, ICW4_8086);
	outb(PIC_SLAVE_DATA, ICW4_8086);
 
   outb(PIC_MASTER_DATA, a1);
   outb(PIC_SLAVE_DATA, a2);

   /* Mask all interrupts for now, except for slave interrupt (IRQ2) */
   for (i = 0; i < 16; i++)
      if (i != 2)
         IRQ_set_mask(i);
}

#define PIC_EOI		0x20		/* End-of-interrupt command code */
 
void PIC_sendEOI(unsigned char irq)
{
	if(irq >= 8)
		outb(PIC_SLAVE_COMMAND,PIC_EOI);
 
	outb(PIC_MASTER_COMMAND,PIC_EOI);
}

void IRQ_set_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        IRQline -= 8;
    }
    value = inb(port) | (1 << IRQline);
    outb(port, value);        
}

void IRQ_clear_mask(unsigned char IRQline) {
    uint16_t port;
    uint8_t value;
 
    if(IRQline < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        IRQline -= 8;
    }
    value = inb(port) & ~(1 << IRQline);
    outb(port, value);        
}

void IRQ_generic_isr(uint32_t irq)
{
   if (irq >= 0x20 && irq <= 0x2F)
      IRQ_clear_mask(irq - PIC_MASTER_REMAP_BASE);
   printk("Received interrupt: 0x%X\n", irq);
}

void IRQ_generic_isr_error(uint32_t irq, uint32_t err)
{
   printk("Received error interrupt: 0x%X 0x%X\n", irq, err);
}

void IRQ_init()
{
   PIC_init();
   IDT_init();

   STI;
}