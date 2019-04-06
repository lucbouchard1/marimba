IMG_FILENAME = fs.img
MAIN_FS_DIR = img
KERNEL_FILENAME = kernel.bin
ASM_SRC = multiboot_header.asm boot.asm
ASM_OBJ = $(ASM_SRC:.asm=.o)
GRUB_CFG=grub.cfg

.PHONY: install_kernel run clean

all: install_kernel

$(IMG_FILENAME):
	./scripts/create_img_file.sh $(IMG_FILENAME) $(MAIN_FS_DIR)
	sudo ./scripts/install_grub.sh $(IMG_FILENAME) $(MAIN_FS_DIR)

$(KERNEL_FILENAME): $(ASM_OBJ)
	x86_64-elf-ld -n -o $(KERNEL_FILENAME) -T linker.ld $^

$(MAIN_FS_DIR): $(KERNEL_FILENAME) $(GRUB_CFG)
	mkdir -p $(MAIN_FS_DIR)/boot/grub
	cp $(GRUB_CFG) $(MAIN_FS_DIR)/boot/grub
	cp $(KERNEL_FILENAME) $(MAIN_FS_DIR)/boot

install_kernel: $(IMG_FILENAME) $(MAIN_FS_DIR)
	sudo ./scripts/install_kernel.sh $(IMG_FILENAME) $(MAIN_FS_DIR)

run: all
	qemu-system-x86_64 -s -drive format=raw,file=fs.img -serial stdio

%.o: %.asm
	nasm -f elf64 -o $@ $<

clean:
	rm -fr *.o $(KERNEL_FILENAME) $(MAIN_FS_DIR) $(IMG_FILENAME)