#ifndef __X86_64_INTERRUPTS_H__
#define __X86_64_INTERRUPTS_H__

void IRQ_x86_64_init();

#define STI asm("sti");
#define CLI asm("cli");

#endif