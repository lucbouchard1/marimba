global isr_normal
global isr_error
extern IRQ_generic_isr
extern IRQ_generic_isr_error
global isr_0

section .text
bits 64
isr_normal:
   call IRQ_generic_isr
   pop rdi
   iretq

isr_error:
   call IRQ_generic_isr_error
   pop rsi
   pop rdi
   add rsp, 4    ; Remove error code from stack
   iretq