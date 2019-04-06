ARCH ?= x86_64
IMG_FILENAME = fs.img
MAIN_FS_DIR = img
KERNEL_FILENAME = kernel.bin
ASM_SRC = multiboot_header.asm boot.asm
ASM_OBJ = $(ASM_SRC:.asm=.o)

LINKER_SCRIPT := src/arch/$(ARCH)/linker.ld
GRUB_CFG := src/arch/$(ARCH)/grub.cfg
ASM_SRC := $(wildcard src/arch/$(ARCH)/*.asm)
ASM_OBJ := $(patsubst src/arch/$(ARCH)/%.asm, \
	build/arch/$(ARCH)/%.o, $(ASM_SRC))

.PHONY: run clean

all: $(IMG_FILENAME)

$(IMG_FILENAME): $(MAIN_FS_DIR)
	@./scripts/create_img_file.sh $(IMG_FILENAME) $(MAIN_FS_DIR)
	@sudo ./scripts/install_kernel.sh $(IMG_FILENAME) $(MAIN_FS_DIR)

$(KERNEL_FILENAME): $(ASM_OBJ)
	x86_64-elf-ld -n -o $(KERNEL_FILENAME) -T $(LINKER_SCRIPT) $^

$(MAIN_FS_DIR): $(KERNEL_FILENAME) $(GRUB_CFG)
	@mkdir -p $(MAIN_FS_DIR)/boot/grub
	@cp $(GRUB_CFG) $(MAIN_FS_DIR)/boot/grub
	@cp $(KERNEL_FILENAME) $(MAIN_FS_DIR)/boot

run: all
	qemu-system-x86_64 -s -drive format=raw,file=fs.img -serial stdio

build/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -f elf64 -o $@ $<

clean:
	rm -fr build $(KERNEL_FILENAME) $(MAIN_FS_DIR) $(IMG_FILENAME)