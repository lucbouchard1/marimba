FS_BLOCK_SIZE=512
FS_BLOCK_COUNT=32768
FS_MAIN_PARTITION_START_BLOCK=2048
FS_MAIN_PARTITION_BLOCK_COUNT=30720
FS_MAIN_PARTITION_START_ADDR=$(echo $FS_BLOCK_SIZE '*' $FS_MAIN_PARTITION_START_BLOCK | bc)
FS_MAIN_MOUNT_POINT=/mnt/fatgrub