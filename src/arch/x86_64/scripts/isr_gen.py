
syscall_irq = 123

isr_normal = """\
global isr_{0}
isr_{0}:
    push rdi
    push rsi
    mov rdi, {0}
    mov rsi, 0
    jmp isr_asm_generic
"""

isr_error = """\
global isr_{0}
isr_{0}:
    push rsi
    mov rsi, [rsp + 8] ; Save error code value
    mov [rsp + 8], rdi ; Overwrite error code with rdi value
    mov rdi, {0}
    jmp isr_asm_generic
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

print('extern isr_asm_generic')
print('section .text')
print('bits 64')
for irq,handler in irq_num_to_handler.items():
    if irq != syscall_irq:
        print(handler.format(irq));