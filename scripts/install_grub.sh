#!/bin/bash

FS_FILENAME=$1
FS_MAIN_DIR=$2
source ./scripts/config.sh

function cleanup () {
   if [[ -v LOOP_DEV_BOOT ]]; then
      losetup -d $LOOP_DEV_BOOT
   fi
   echo "Done!"
   exit $1
}

if [ ! -f $FS_FILENAME ]; then
   dd if=/dev/zero of=$FS_FILENAME bs=$FS_BLOCK_SIZE count=$FS_BLOCK_COUNT || cleanup -1
   parted $FS_FILENAME mklabel msdos || cleanup -1
   parted $FS_FILENAME mkpart primary fat32 ${FS_MAIN_PARTITION_START_BLOCK}s \
         ${FS_MAIN_PARTITION_BLOCK_COUNT}s || cleanup -1
   parted $FS_FILENAME set 1 boot on || cleanup -1
fi

LOOP_DEV_BOOT=$(losetup -f) || cleanup -1
losetup $LOOP_DEV_BOOT $FS_FILENAME || cleanup -1
grub-install -d /usr/lib/grub/i386-pc --root-directory=$FS_MAIN_MOUNT_POINT --no-floppy \
      --modules="normal part_msdos ext2 multiboot" $LOOP_DEV_BOOT || cleanup -1
cleanup 0