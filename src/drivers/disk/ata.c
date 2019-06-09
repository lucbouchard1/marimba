#include "../../files.h"
#include "../../printk.h"
#include "../../io.h"
#include "../../klog.h"
#include "../../string.h"
#include "../pci.h"

#define ATA_LEGACY_PRIMARY_BASE 0x1F0
#define ATA_LEGACY_PRIMARY_CNTL 0x3F6
#define ATA_LEGACY_SECONDARY_BASE 0x170
#define ATA_LEGACY_SECONDARY_CNTL 0x376

#define DATA_REG_OFF 0
#define ERR_REG_OFF 1
#define FEAT_REG_OFF 1
#define SECT_CNT_REG_OFF 2
#define CYL_LOW_REG_OFF 4
#define CYL_HIGH_REG_OFF 5
#define DRIVE_REG_OFF 6
#define STATUS_REG_OFF 7
#define CMD_REG_OFF 7

#define ATA_SELECT_MASTER 0xA0
#define ATA_SELECT_SLAVE 0xB0

#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATA_BLOCK_SIZE 512

int ata_probe(struct PCIDriver *driver);

static struct ATAPCIDriver {
   struct PCIDriver driv;
   uint32_t base, cntl_base, master;
   uint8_t irq, slave;
   struct ATARequest *req_head, *req_tail;
} pci_ata_dev = {
   .driv.bdev = {
      .name = "ata",
      .type = MASS_STORAGE,
      .blk_size = ATA_BLOCK_SIZE,
   },
   .driv.id = {
      .class = 1,
      .sub_class = 1
   },
   .driv.probe = &ata_probe,
   .driv.dev = NULL
};

static void ata_400ns_delay(struct ATAPCIDriver *driv)
{
   inb(driv->base + STATUS_REG_OFF);
   inb(driv->base + STATUS_REG_OFF);
   inb(driv->base + STATUS_REG_OFF);
   inb(driv->base + STATUS_REG_OFF);
}

static void ata_wait_not_busy(struct ATAPCIDriver *driv)
{
   while (inb(driv->base + STATUS_REG_OFF) & ATA_SR_BSY);
}

static int ata_check_err(struct ATAPCIDriver *driv)
{
   return inb(driv->base + STATUS_REG_OFF) & ATA_SR_ERR;
}

static void ata_wait_data_ready(struct ATAPCIDriver *driv)
{
   while (!(inb(driv->base + STATUS_REG_OFF) & ATA_SR_DRQ));
}

static void ata_read_block(struct ATAPCIDriver *ata, uint16_t *buff)
{
   int i;

   ata_wait_data_ready(ata);

   for (i = 0; i < ATA_BLOCK_SIZE/2; i++)
      buff[i] = inw(ata->base + DATA_REG_OFF);

   ata_400ns_delay(ata);
}

int ata_probe(struct PCIDriver *driver)
{
   struct PCIConfigHeader_0 hdr;
   struct ATAPCIDriver *ata = (struct ATAPCIDriver *)driver;
   uint16_t iden[ATA_BLOCK_SIZE/2];

   PCI_read_config_header_0(driver->dev, &hdr);

   if (hdr.bar0 == 0 || hdr.bar0 == 1)
      ata->base = ATA_LEGACY_PRIMARY_BASE;
   else
      ata->base = hdr.bar0;

   if (hdr.bar1 == 0 || hdr.bar1 == 1)
      ata->cntl_base = ATA_LEGACY_PRIMARY_CNTL;
   else
      ata->cntl_base = hdr.bar1;

   outb(ata->base + DRIVE_REG_OFF, ATA_SELECT_MASTER);
   ata_400ns_delay(ata);
   outb(ata->base + SECT_CNT_REG_OFF, 0);
   outb(ata->base + CYL_LOW_REG_OFF, 0);
   outb(ata->base + CYL_HIGH_REG_OFF, 0);
   outb(ata->base + CMD_REG_OFF, ATA_CMD_IDENTIFY);

   if (!inb(ata->base + STATUS_REG_OFF)) {
      klog(KLOG_LEVEL_WARN, "ata device not found");
      return -1;
   }

   ata_wait_not_busy(ata);
   if (ata_check_err(ata)) {
      klog(KLOG_LEVEL_WARN, "ata device uses unsupported ATAPI interface");
      return -1;
   }
   ata_read_block(ata, iden);

   if (!(iden[83] & (1<<10))) {
      klog(KLOG_LEVEL_WARN, "ata uses unsupported 28-bit LDA");
      return -1;
   }

   ata->driv.bdev.total_len = ATA_BLOCK_SIZE * *((uint64_t *)(iden + 100));
   ata->irq = 14;
   ata->master = hdr.bar4;
   ata->slave = 0;
   return 0;
}

int ata_init_module()
{
   PCI_register((struct PCIDriver *)&pci_ata_dev);
   return 0;
}