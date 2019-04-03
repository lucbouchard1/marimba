include config.mk

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
	dd if=/dev/zero of=$(FS_FILENAME) bs=$(FS_BLOCK_SIZE) count=$(FS_BLOCK_COUNT)
	parted $(FS_FILENAME) mklabel msdos
	parted $(FS_FILENAME) mkpart primary fat32 $(FS_MAIN_PARTITION_START_BLOCK)s \
			$(FS_MAIN_PARTITION_BLOCK_COUNT)s
	parted $(FS_FILENAME) set 1 boot on
	$(eval LOOP_DEV_BOOT := $(shell losetup -f))
	losetup $(LOOP_DEV_BOOT) $(FS_FILENAME)
	$(eval LOOP_DEV_MAIN := $(shell losetup -f))
	losetup $(LOOP_DEV_MAIN) $(FS_FILENAME) -o $(FS_MAIN_PARTITION_START_ADDR)
	mkdosfs -F32 -f 2 $(LOOP_DEV_MAIN)
	mount $(LOOP_DEV_MAIN) $(FS_MAIN_MOUNT_POINT)
	grub-install --root-directory=$(FS_MAIN_MOUNT_POINT) --no-floppy \
			--modules="normal part_msdos ext2 multiboot" $(LOOP_DEV_BOOT)
	cp -r img/* $(FS_MAIN_MOUNT_POINT)
	umount $(FS_MAIN_MOUNT_POINT)
	losetup -d $(LOOP_DEV_BOOT)
	losetup -d $(LOOP_DEV_MAIN)