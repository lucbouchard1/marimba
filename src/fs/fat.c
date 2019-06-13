#include "../klog.h"
#include "../kmalloc.h"
#include "fat.h"

struct FAT {
   int sects_per_clust, rsvd_sects, root_clust;
   int num_fats, tot_sects, sects_per_fat;
   int sect_size, num_fat_entires;
   uint32_t *table;
};

struct FATDir {
   struct FAT *fat;
   int start_clust;
};

struct FAT_BPB {
   uint8_t jmp[3];
   char oem_id[8];
   uint16_t bytes_per_sector;
   uint8_t sectors_per_cluster;
   uint16_t reserved_sectors;
   uint8_t num_fats;
   uint16_t num_dirents;
   uint16_t tot_sectors;
   uint8_t mdt;
   uint16_t sectors_per_fat;
   uint16_t sectors_per_track;
   uint16_t num_heads;
   uint32_t num_hidden_sectors;
   uint32_t large_sector_count;
} __attribute__((packed));

struct FAT32 {
   struct FAT_BPB bpb;
   uint32_t sectors_per_fat;
   uint16_t flags;
   uint8_t major_vers;
   uint8_t minor_vers;
   uint32_t root_cluster_number;
   uint16_t fsinfo_sector;
   uint16_t backup_boot_sector;
   uint8_t zero[12];
   uint8_t drive_num;
   uint8_t nt_flags;
   uint8_t signature;
   uint32_t serial_num;
   char label[11];
   char sys_id[8];
   uint8_t boot_code[420];
   uint8_t boot_sig[2];
} __attribute__((packed));

struct FATDirEnt {
   char name[11];
   uint8_t attr;
   uint8_t nt;
   uint8_t ct_tenths;
   uint16_t ct;
   uint16_t cd;
   uint16_t ad;
   uint16_t cluster_hi;
   uint16_t mt;
   uint16_t md;
   uint16_t cluster_lo;
   uint32_t size;
} __attribute__((packed));

struct FATLongDirEnt {
   uint8_t order;
   uint16_t first[5];
   uint8_t attr;
   uint8_t type;
   uint8_t checksum;
   uint16_t middle[6];
   uint16_t zero;
   uint16_t last[2];
} __attribute__((packed));

uint32_t fat32_next_cluster(struct FAT *fat, int clust)
{
   int i;

   for (i = 0; i < fat->num_fat_entires; i++) {
      if (fat->table[i] == clust)
         return fat->table[i+1];
   }

   return 0;
}

int fat32_parse_dir(struct INode *new, int dir_clust)
{

}

int FS_register_fat32(struct PartBlockDev *part)
{
   struct FAT32 fat;
   struct INode *root_inode;
   struct FATDir *root;
   struct FAT *tbl;
   int i;

   klog(KLOG_LEVEL_WARN, "FAT 32!");

   if (part->dev.read_block(&part->dev, 0, &fat) < 0) {
      klog(KLOG_LEVEL_WARN, "failed to read from FAT 32 filesystem");
      return -1;
   }

   /* TODO: Use partition label stored in FAT32 */

   if (fat.signature != 0x28 && fat.signature != 0x29) {
      klog(KLOG_LEVEL_WARN, "FAT 32 filesystem on part %s invalid",
         part->dev.name);
      return -1;
   }

   tbl = kmalloc(sizeof(struct FAT));
   if (!tbl)
      return -1;

   tbl->rsvd_sects = fat.bpb.reserved_sectors;
   tbl->num_fats = fat.bpb.num_fats;
   tbl->sects_per_clust = fat.bpb.sectors_per_cluster;
   tbl->tot_sects = fat.bpb.tot_sectors ? fat.bpb.tot_sectors : fat.sectors_per_fat;
   tbl->sects_per_fat = fat.bpb.sectors_per_fat ? fat.bpb.sectors_per_fat : fat.sectors_per_fat;
   tbl->root_clust = fat.root_cluster_number;
   tbl->sect_size = part->dev.blk_size;
   tbl->num_fat_entires = (tbl->num_fats * tbl->sects_per_fat * tbl->sect_size)/4;

   tbl->table = kmalloc(tbl->num_fats * tbl->sects_per_fat * tbl->sect_size);
   if (!tbl->table)
      return -1;

   for (i = 0; i < tbl->num_fats*tbl->sects_per_fat; i++) {
      part->dev.read_block(&part->dev, i + (1 + tbl->rsvd_sects),
            &tbl->table[i * tbl->sect_size]);
   }

   root_inode = kmalloc(sizeof(struct INode));
   if (!root_inode)
      return -1;

   root = kmalloc(sizeof(struct FATDir));
   if (!root) {
      kfree(root_inode);
      return -1;
   }
   root->fat = tbl;
   root->start_clust = tbl->root_clust;
   root_inode->private = root;

   return 0;
}