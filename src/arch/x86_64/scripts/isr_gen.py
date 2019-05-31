
syscall_irq = 123

isr_normal = """\
global isr_{0}
isr_{0}:
    push r10
    push r11
    push rdi
    push rsi
    push rdx
    push rcx
    push r8
    push r9
    push rax
    mov rdi, {0}
    mov rsi, 0
    jmp isr_normal
"""

isr_error = """\
global isr_{0}
isr_{0}:
    push rsi
    mov rsi, [rsp + 8]
    push r10
    push r11
    push rdi
    push rdx
    push rcx
    push r8
    push r9
    push rax
    mov rdi, {0}
    jmp isr_error
"""

irq_num_to_handler = {k:isr_normal for k in range(256)}
irq_num_to_handler[0x8] = isr_error
irq_num_to_handler[0xA] = isr_error
irq_num_to_handler[0xB] = isr_error
irq_num_to_handler[0xC] = isr_error
irq_num_to_handler[0xD] = isr_error
irq_num_to_handler[0xE] = isr_error
irq_num_to_handler[0x11] = isr_error
irq_num_to_handler[0x1E] = isr_error

print('extern isr_normal')
print('extern isr_error')
print('section .text')
print('bits 64')
for irq,handler in irq_num_to_handler.items():
    if irq != syscall_irq:
        print(handler.format(irq));