global isr_normal
global isr_error
global isr_123
extern IRQ_generic_isr
extern syscall_handler

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
   call IRQ_generic_isr
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

isr_123:
   push r10
   push r11
   push rdi
   push rsi
   push rdx
   push rcx
   push r8
   push r9
   push rax
   mov rdi, 123
   mov rsi, rbx
   jmp isr_normal