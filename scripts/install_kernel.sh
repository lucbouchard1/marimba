#!/bin/bash

FS_FILENAME=$1
FS_MAIN_DIR=$2
source ./scripts/config.sh

function cleanup () {
   umount $FS_MAIN_MOUNT_POINT
   if [[ -v LOOP_DEV_MAIN ]]; then
      losetup -d $LOOP_DEV_MAIN
   fi
   if [[ -v LOOP_DEV_BOOT ]]; then
      losetup -d $LOOP_DEV_BOOT
   fi
   echo "Done!"
   exit $1
}

if [ ! -f $FS_FILENAME ]; then
   echo "create_img_file.sh must be run before install_kernel.sh"
   exit -1
fi

# Install kernel
LOOP_DEV_MAIN=$(losetup -f) || cleanup -1
losetup $LOOP_DEV_MAIN $FS_FILENAME -o $FS_MAIN_PARTITION_START_ADDR || cleanup -1
mkdosfs -F32 -f 2 $LOOP_DEV_MAIN || cleanup -1
mount $LOOP_DEV_MAIN $FS_MAIN_MOUNT_POINT
cp -r img/* $FS_MAIN_MOUNT_POINT || cleanup -1

# Install grub
LOOP_DEV_BOOT=$(losetup -f) || cleanup -1
losetup $LOOP_DEV_BOOT $FS_FILENAME || cleanup -1
grub-install -d /usr/lib/grub/i386-pc --root-directory=$FS_MAIN_MOUNT_POINT --no-floppy \
      --modules="normal part_msdos ext2 multiboot" $LOOP_DEV_BOOT || cleanup -1
cleanup 0