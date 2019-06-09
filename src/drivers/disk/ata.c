#include "../../files.h"
#include "../../printk.h"
#include "../pci.h"

int ata_probe(struct PCIDriver *driver);

static struct ATAPCIDriver {
   struct PCIDriver driv;
   uint16_t ata_base, ata_master;
   uint8_t slave, irq;
   struct ATARequest *req_head, *req_tail;
} pci_ata_dev = {
   .driv.bdev = {
      .name = "ata",
      .type = MASS_STORAGE,
   },
   .driv.id = {
      .class = 1,
      .sub_class = 1
   },
   .driv.probe = &ata_probe,
   .driv.dev = NULL
};

int ata_probe(struct PCIDriver *driver)
{
   printk("probing ata device!\n");
   printk("   class: %d\n", driver->id.class);
   printk("   subclass: %d\n", driver->id.sub_class);
   return 0;
}

int ata_init_module()
{
   PCI_register((struct PCIDriver *)&pci_ata_dev);
   return 0;
}