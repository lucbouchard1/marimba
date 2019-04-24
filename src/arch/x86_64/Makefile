
ISR_FILENAME = GENERATED_ASM_isr.asm
IDT_FILENAME = GENERATED_C_idt.c

ISR_SCRIPT = scripts/isr_gen.py
IDT_SCRIPT = scripts/idt_gen.py

all: $(ISR_FILENAME) $(IDT_FILENAME)

$(ISR_FILENAME): $(ISR_SCRIPT)
	python $< > $@

$(IDT_FILENAME): $(IDT_SCRIPT)
	python $< > $@
