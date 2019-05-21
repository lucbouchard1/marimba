global isr_normal
global isr_error
extern IRQ_generic_isr
extern IRQ_generic_isr_error
global isr_0

section .text
bits 64
isr_normal:
   call IRQ_generic_isr
   pop rax
   pop r9
   pop r8
   pop rcx
   pop rdx
   pop rsi
   pop rdi
   pop r11
   pop r10
   iretq

isr_error:
   call IRQ_generic_isr_error
   pop rax
   pop r9
   pop r8
   pop rcx
   pop rdx
   pop rdi
   pop r11
   pop r10
   pop rsi
   add rsp, 8    ; Remove error code from stack
   iretq