#include "../../printk.h"
#include "../../io.h"
#include "../../klog.h"
#include "../../string.h"
#include "../../interrupts.h"
#include "../../proc.h"
#include "../../list.h"
#include "../../kmalloc.h"
#include "../pci.h"

#define ATA_LEGACY_PRIMARY_BASE 0x1F0
#define ATA_LEGACY_PRIMARY_CNTL 0x3F6
#define ATA_LEGACY_SECONDARY_BASE 0x170
#define ATA_LEGACY_SECONDARY_CNTL 0x376

#define ATA_PRIMARY_IRQ (0x20 + 14)
#define ATA_SECONDARY_IRQ (0x20 + 15)

#define DATA_REG_OFF 0
#define ERR_REG_OFF 1
#define FEAT_REG_OFF 1
#define SECT_CNT_REG_OFF 2
#define LBA_LOW_OFF 3
#define LBA_MID_OFF 4
#define LBA_HIGH_OFF 5
#define DRIVE_REG_OFF 6
#define STATUS_REG_OFF 7
#define CMD_REG_OFF 7

#define ATA_SELECT_MASTER 0xE0
#define ATA_SELECT_SLAVE 0xF0

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

int ata_probe(struct PCIDevice *driver);

struct ATARequest {
   struct ListHeader list;
   uint64_t lba;
   int read;
};

static struct PCIDriver ata_pci_driver = {
   .id = {
      .class = 1,
      .sub_class = 1
   },
   .probe = &ata_probe,
};

struct ATADevice {
   struct BlockDev dev;
   struct PCIDriver driv;
   uint32_t base, cntl_base, master;
   uint8_t irq, slave;
   struct ProcessQueue blocked;
   struct LinkedList requests;
};

static void ata_400ns_delay(struct ATADevice *driv)
{
   inb(driv->base + STATUS_REG_OFF);
   inb(driv->base + STATUS_REG_OFF);
   inb(driv->base + STATUS_REG_OFF);
   inb(driv->base + STATUS_REG_OFF);
}

static void ata_wait_not_busy(struct ATADevice *driv)
{
   while (inb(driv->cntl_base) & ATA_SR_BSY);
}

static int ata_check_err(struct ATADevice *driv)
{
   return inb(driv->cntl_base) & ATA_SR_ERR;
}

static int ata_check_data(struct ATADevice *driv)
{
   return inb(driv->cntl_base) & ATA_SR_DRQ;
}

static void ata_wait_data_ready(struct ATADevice *driv)
{
   while (!(inb(driv->cntl_base) & ATA_SR_DRQ));
}

static void ata_read(struct ATADevice *ata, uint16_t *buff)
{
   int i;

   ata_wait_data_ready(ata);

   for (i = 0; i < ATA_BLOCK_SIZE/2; i++)
      buff[i] = inw(ata->base + DATA_REG_OFF);
}

static void ata_isr(int irq, int err, void *arg)
{
   struct ATADevice *ata = (struct ATADevice *)arg;

   klog(KLOG_LEVEL_DEBUG, "ata interrupt");

   /* Read status reg to clear interrupt */
   inb(ata->base + STATUS_REG_OFF);

   if (ata_check_err(ata) || !ata_check_data(ata)) {
      klog(KLOG_LEVEL_WARN, "ata error detected: 0x%x", inb(ata->base + ERR_REG_OFF));
      return;
   }

   PROC_unblock_head(&ata->blocked);
}

static void ata_write_lba(struct ATADevice *ata, uint64_t lba, uint16_t sectorcount)
{
   outb(ata->base + SECT_CNT_REG_OFF, (sectorcount>>8) & 0xff);
   outb(ata->base + LBA_LOW_OFF, (lba >> 24) & 0xff);
   outb(ata->base + LBA_MID_OFF, (lba >> 32) & 0xff);
   outb(ata->base + LBA_HIGH_OFF, (lba >> 40) & 0xff);
   outb(ata->base + SECT_CNT_REG_OFF, sectorcount & 0xff);
   outb(ata->base + LBA_LOW_OFF, (lba >> 0) & 0xff);
   outb(ata->base + LBA_MID_OFF, (lba >> 8) & 0xff);
   outb(ata->base + LBA_HIGH_OFF, (lba >> 16) & 0xff);
}

int ata_read_block(struct BlockDev *dev, sect_t blk_num, void *dst)
{
   struct ATADevice *ata = container_of(dev, struct ATADevice, dev);
   struct ATARequest req, *next;

   req.lba = blk_num;
   req.read = 1;

   IRQ_disable();
   if (LL_empty(&ata->requests)) {
      ata_write_lba(ata, req.lba, 1);
      outb(ata->base + CMD_REG_OFF, ATA_CMD_READ_PIO_EXT);
   }
   LL_enqueue(&ata->requests, &req);
   PROC_block_on(&ata->blocked, 1);

   /* Read block data */
   if (!ata_check_data(ata)) {
      klog(KLOG_LEVEL_WARN, "ata read failed");
      return -1;
   }
   LL_dequeue(&ata->requests);
   ata_read(ata, dst);

   /* Initiate next read */
   next = LL_head(&ata->requests);
   if (next) {
      ata_write_lba(ata, next->lba, 1);
      outb(ata->base + CMD_REG_OFF, ATA_CMD_READ_PIO_EXT);
   }
   return 0;
}

int ata_probe(struct PCIDevice *dev)
{
   struct PCIConfigHeader_0 hdr;
   struct ATADevice *ata;
   uint16_t iden[ATA_BLOCK_SIZE/2];

   ata = kmalloc(sizeof(struct ATADevice));
   if (!ata)
      return -1;

   LL_init(&ata->requests, offsetof(struct ATARequest, list));
   PROC_queue_init(&ata->blocked);

   PCI_read_config_header_0(dev, &hdr);

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
   ata_write_lba(ata, 0, 1);
   outb(ata->base + CMD_REG_OFF, ATA_CMD_IDENTIFY);

   if (!inb(ata->base + STATUS_REG_OFF)) {
      klog(KLOG_LEVEL_WARN, "ata device not found");
      kfree(ata);
      return -1;
   }

   ata_wait_not_busy(ata);
   if (ata_check_err(ata)) {
      klog(KLOG_LEVEL_WARN, "ata device uses unsupported ATAPI interface");
      kfree(ata);
      return -1;
   }
   ata_read(ata, iden);

   if (!(iden[83] & (1<<10))) {
      klog(KLOG_LEVEL_WARN, "ata uses unsupported 28-bit LBA");
      kfree(ata);
      return -1;
   }

   ata->dev.total_len = ATA_BLOCK_SIZE * *((uint64_t *)(iden + 100));
   ata->dev.blk_size = ATA_BLOCK_SIZE;
   ata->dev.type = MASS_STORAGE;
   ata->dev.read_block = &ata_read_block;
   ata->dev.name = "ata"; // TODO: Made this change dynamically

   ata->irq = ATA_PRIMARY_IRQ;
   ata->master = hdr.bar4;
   ata->slave = 0;

   IRQ_set_handler(ata->irq, ata_isr, ata);
   IRQ_clear_mask(ata->irq);

   /* Enable interrupts */
   outb(ata->cntl_base, 0x00);

   BLK_register(&ata->dev);
   return 0;
}

int ata_init_module()
{
   PCI_register(&ata_pci_driver);
   return 0;
}