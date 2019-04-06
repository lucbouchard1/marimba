ARCH ?= x86_64
MARIMBA_BOOTABLE_IMG = marimba.img
MARIMBA_BASE_FS = build/img
KERNEL_FILENAME = build/kernel.bin
ASM_SRC = multiboot_header.asm boot.asm
ASM_OBJ = $(ASM_SRC:.asm=.o)

LINKER_SCRIPT := src/arch/$(ARCH)/linker.ld
GRUB_CFG := src/arch/$(ARCH)/grub.cfg
ASM_SRC := $(wildcard src/arch/$(ARCH)/*.asm)
ASM_OBJ := $(patsubst src/arch/$(ARCH)/%.asm, \
	build/arch/$(ARCH)/%.o, $(ASM_SRC))

.PHONY: run clean

all: $(MARIMBA_BOOTABLE_IMG)

$(MARIMBA_BOOTABLE_IMG): $(MARIMBA_BASE_FS)
	@./scripts/create_img_file.sh $(MARIMBA_BOOTABLE_IMG) $(MARIMBA_BASE_FS)
	@sudo ./scripts/install_kernel.sh $(MARIMBA_BOOTABLE_IMG) $(MARIMBA_BASE_FS)

$(KERNEL_FILENAME): $(ASM_OBJ)
	x86_64-elf-ld -n -o $(KERNEL_FILENAME) -T $(LINKER_SCRIPT) $^

$(MARIMBA_BASE_FS): $(KERNEL_FILENAME) $(GRUB_CFG)
	@mkdir -p $(MARIMBA_BASE_FS)/boot/grub
	@cp $(GRUB_CFG) $(MARIMBA_BASE_FS)/boot/grub
	@cp $(KERNEL_FILENAME) $(MARIMBA_BASE_FS)/boot

run: all
	qemu-system-x86_64 -s -drive format=raw,file=$(MARIMBA_BOOTABLE_IMG) -serial stdio

build/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 -o $@ $<

clean:
	rm -fr build $(KERNEL_FILENAME) $(MARIMBA_BASE_FS) $(MARIMBA_BOOTABLE_IMG)