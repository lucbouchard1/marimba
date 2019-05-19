ARCH ?= x86_64
MARIMBA_BOOTABLE_IMG = marimba.img
MARIMBA_BASE_FS = build/img
KERNEL_FILENAME = build/kernel.bin

TARGET_CC = x86_64-elf-gcc
TARGET_LD = x86_64-elf-ld
CFLAGS += -g -Wall -Werror

LINKER_SCRIPT := src/arch/$(ARCH)/scripts/linker.ld
GRUB_CFG := src/arch/$(ARCH)/grub.cfg
C_SRC = $(shell find src/ -path src/arch -prune -o -print | grep .c$$) \
		$(shell make --no-print-directory -C src/arch/$(ARCH) c_src)
C_OBJ = $(patsubst src/%.c, build/%.o, $(C_SRC))
ASM_SRC = $(shell make --no-print-directory -C src/arch/$(ARCH) asm_src)
ASM_OBJ = $(patsubst src/arch/$(ARCH)/%.asm, \
		build/arch/$(ARCH)/%.o, $(ASM_SRC))

TESTS_C_SRC := $(wildcard tests/*.c)
TESTS_C_OBJ := $(patsubst tests/%.c, tests/%.o, $(TESTS_C_SRC)) \
		tests/string.o tests/utils.o
TESTS_EXECUTABLE = run_tests

.PHONY: run clean test arch

all: arch test $(MARIMBA_BOOTABLE_IMG)

arch:
	make -C src/arch/$(ARCH)

$(MARIMBA_BOOTABLE_IMG): $(MARIMBA_BASE_FS)
	@./scripts/create_img_file.sh $(MARIMBA_BOOTABLE_IMG) $(MARIMBA_BASE_FS)
	@sudo ./scripts/install_kernel.sh $(MARIMBA_BOOTABLE_IMG) $(MARIMBA_BASE_FS)

$(MARIMBA_BASE_FS): $(KERNEL_FILENAME) $(GRUB_CFG)
	@mkdir -p $(MARIMBA_BASE_FS)/boot/grub
	@touch $(MARIMBA_BASE_FS)
	@cp $(GRUB_CFG) $(MARIMBA_BASE_FS)/boot/grub
	@cp $(KERNEL_FILENAME) $(MARIMBA_BASE_FS)/boot

$(KERNEL_FILENAME): $(ASM_OBJ) $(C_OBJ)
	$(TARGET_LD) -n -o $(KERNEL_FILENAME) -T $(LINKER_SCRIPT) $^

$(TESTS_EXECUTABLE): $(TESTS_C_OBJ)
	$(CC) $(CFLAGS) $^ -o $(TESTS_EXECUTABLE)

test: $(TESTS_EXECUTABLE)
	./$(TESTS_EXECUTABLE)

run: all
	qemu-system-x86_64 -s -drive format=raw,file=$(MARIMBA_BOOTABLE_IMG) -serial stdio

debug: all
	qemu-system-x86_64 -s -drive format=raw,file=$(MARIMBA_BOOTABLE_IMG) -serial stdio &
	gdb -x scripts/gdbinit

build/arch/$(ARCH)/%.o: src/arch/$(ARCH)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -g -f elf64 -o $@ $<

build/%.o: src/%.c
	@mkdir -p $(shell dirname $@)
	$(TARGET_CC) -DARCH=$(ARCH) $(CFLAGS) -mno-red-zone -ffreestanding -c -o $@ $<

tests/%.o: src/%.c
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -fr build tests/*.o $(TESTS_EXECUTABLE) $(KERNEL_FILENAME) $(MARIMBA_BASE_FS) $(MARIMBA_BOOTABLE_IMG)
	make -C src/arch/$(ARCH) clean
