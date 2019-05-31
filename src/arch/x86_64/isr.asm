global isr_asm_generic
global isr_123
extern IRQ_generic_isr

section .text
bits 64
isr_asm_generic:
   push r10
   push r11
   push rdx
   push rcx
   push r8
   push r9
   push rax
   push rbp
   mov rdx, rsp
   call IRQ_generic_isr
   pop rbp
   pop rax
   pop r9
   pop r8
   pop rcx
   pop rdx
   pop r11
   pop r10
   pop rsi
   pop rdi
   iretq

isr_123:
   push rdi
   push rsi
   mov rdi, 123
   mov rsi, rbx
   jmp isr_asm_generic