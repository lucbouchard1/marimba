#!/bin/bash

FS_FILENAME=$1
FS_MAIN_DIR=$2
source ./scripts/config.sh

if [ ! -f $FS_FILENAME ]; then
   dd if=/dev/zero of=$FS_FILENAME bs=$FS_BLOCK_SIZE count=$FS_BLOCK_COUNT || cleanup -1
   parted $FS_FILENAME mklabel msdos || cleanup -1
   parted $FS_FILENAME mkpart primary fat32 ${FS_MAIN_PARTITION_START_BLOCK}s \
         ${FS_MAIN_PARTITION_BLOCK_COUNT}s || cleanup -1
   parted $FS_FILENAME set 1 boot on || cleanup -1
fi