#ifndef __X86_64_INTERRUPTS_H__
#define __X86_64_INTERRUPTS_H__

#define DIV_BY_ZERO_IRQ 0x0
#define DEBUG_IRQ 0x1
#define NON_MASKABLE_IRQ 0x2
#define BREAKPOINT_IRQ 0x3
#define OVERFLOW_IRQ 0x4
#define BOUND_RANGE_EXCEEDED_IRQ 0x5
#define INVALID_OPCODE_IRQ 0x6
#define DEV_NOT_AVAIL_IRQ 0x7
#define DOUBLE_FAULT_IRQ 0x8
#define INVALID_TSS_IRQ 0xA
#define SEGMENT_NOT_PRESENT_IRQ 0xB
#define STACK_SEGMENT_FAULT_IRQ 0xC
#define GENERAL_PROTECTION_FAULT_IRQ 0xD
#define PAGE_FAULT_IRQ 0xE

void IRQ_x86_64_init();

#define STI asm("sti");
#define CLI asm("cli");

#endif