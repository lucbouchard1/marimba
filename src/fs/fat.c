#include "../klog.h"
#include "fat.h"

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

int FS_register_fat32(struct PartBlockDev *part)
{
   struct FAT32 fat;
   // int sects_per_clust, rsvd_sects, root_clust;
   // int num_fats, tot_sects, sects_per_fat;

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

   // rsvd_sects = fat.bpb.reserved_sectors;
   // num_fats = fat.bpb.num_fats;
   // sects_per_clust = fat.bpb.sectors_per_cluster;
   // tot_sects = fat.bpb.tot_sectors ? fat.bpb.tot_sectors : fat.sectors_per_fat;
   // sects_per_fat = fat.bpb.sectors_per_fat ? fat.bpb.sectors_per_fat : fat.sectors_per_fat;
   // root_clust = fat.root_cluster_number;

   return 0;
}