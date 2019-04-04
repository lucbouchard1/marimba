
all: multiboot_header.o boot.o
	x86_64-elf-ld -n -o kernel.bin -T linker.ld $^
	mkdir -p img/boot/grub
	cp grub.cfg img/boot/grub
	cp kernel.bin img/boot

multiboot_header.o: multiboot_header.asm
	nasm -f elf64 multiboot_header.asm

boot.o: boot.asm
	nasm -f elf64 boot.asm

filesystem:
	sudo ./scripts/create_fs.sh

run: all filesystem
	qemu-system-x86_64 -s -drive format=raw,file=fs.img -serial stdio