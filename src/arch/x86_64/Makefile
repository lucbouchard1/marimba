DIR = src/arch/x86_64/

C_SRC = interrupts.c hw_init.c atomic.c
ASM_SRC = boot.asm isr.asm long_mode_init.asm multiboot_header.asm stack.asm
ISR_FILENAME = GENERATED_ASM_isr.asm
IDT_FILENAME = GENERATED_C_idt.c

ISR_SCRIPT = scripts/isr_gen.py
IDT_SCRIPT = scripts/idt_gen.py

all: $(ISR_FILENAME) $(IDT_FILENAME)

$(ISR_FILENAME): $(ISR_SCRIPT)
	python $< > $@

$(IDT_FILENAME): $(IDT_SCRIPT)
	python $< > $@

.PHONY: clean c_src asm_src

c_src:
	@echo $(DIR)$(IDT_FILENAME) $(addprefix $(DIR),$(C_SRC))

asm_src:
	@echo $(DIR)$(ISR_FILENAME) $(addprefix $(DIR),$(ASM_SRC))

clean:
	rm -fr $(ISR_FILENAME) $(IDT_FILENAME)