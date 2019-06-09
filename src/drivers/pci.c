#include "pci.h"
#include "../klog.h"
#include "../io.h"
#include "../list.h"
#include "../kmalloc.h"
#include "../files.h"

struct PCIDevice {
   struct ListHeader list;
   uint8_t bus;
   uint8_t dev;
   uint8_t func;
   struct PCIConfigHeader hdr;
};

static struct PCI {
   struct LinkedList pci_dev_list;
} pci_state = {
   .pci_dev_list = LINKED_LIST_INIT(pci_state.pci_dev_list, struct PCIDevice, list)
};

uint32_t pci_read_config(uint8_t bus, uint8_t dev, uint8_t func,
      uint8_t offset)
{
   uint32_t address, lbus  = (uint32_t)bus, lslot = (uint32_t)dev;
   uint32_t lfunc = (uint32_t)func;

   /* create configuration address */
   address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

   /* write out the address and return the data */
   outl(PCI_CONFIG_ADDR, address);
   return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_config_vendor(uint8_t bus, uint8_t dev, uint8_t func)
{
   uint32_t tmp;

   tmp = pci_read_config(bus, dev, func, 0);
   return (uint16_t)(tmp & 0xffff);
}

void pci_read_config_header(struct PCIConfigHeader *hdr,
      uint8_t bus, uint8_t dev, uint8_t func)
{
   uint32_t *rhdr = (uint32_t *)hdr;
   uint8_t offset;

   for (offset = 0; offset < 4; offset++)
      rhdr[offset] = pci_read_config(bus, dev, func, offset*4);
}

int pci_enum_device(uint8_t bus, uint8_t dev)
{
   struct PCIConfigHeader hdr;
   struct PCIDevice *new;
   uint8_t func;

   if (pci_read_config_vendor(bus, dev, 0) == 0xffff)
      return 0;

   for (func = 0; func < 4; func++) {
      pci_read_config_header(&hdr, bus, dev, func);
      if (hdr.vendor_id == 0xffff)
         return 1;
      new = kmalloc(sizeof(struct PCIDevice));
      if (!new)
         return -1;
      new->bus = bus;
      new->dev = dev;
      new->func = func;
      new->hdr = hdr;
      LL_enqueue(&pci_state.pci_dev_list, new);
      klog(KLOG_LEVEL_INFO, "enumerated PCI device with class 0x%X on bus %d device %d func %d",
            new->hdr.class, new->bus, new->dev, new->func);
   }

   return 1;
}

// int PCI_register(struct BlockDevice *dev, )
// {

// }

int PCI_enum()
{
   uint16_t bus;
   uint8_t device;

   for (bus = 0; bus < 256; bus++)
      for (device = 0; device < 32; device++)
         pci_enum_device(bus, device);

   return 0;
}