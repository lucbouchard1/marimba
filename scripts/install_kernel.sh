#!/bin/bash

FS_FILENAME=$1
FS_MAIN_DIR=$2
source ./scripts/config.sh

function cleanup () {
   umount $FS_MAIN_MOUNT_POINT
   if [[ -v LOOP_DEV_MAIN ]]; then
      losetup -d $LOOP_DEV_MAIN
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


LOOP_DEV_MAIN=$(losetup -f) || cleanup -1
losetup $LOOP_DEV_MAIN $FS_FILENAME -o $FS_MAIN_PARTITION_START_ADDR || cleanup -1
mkdosfs -F32 -f 2 $LOOP_DEV_MAIN || cleanup -1
mount $LOOP_DEV_MAIN $FS_MAIN_MOUNT_POINT
cp -r ${FS_MAIN_DIR}/* $FS_MAIN_MOUNT_POINT || cleanup -1
cleanup 0