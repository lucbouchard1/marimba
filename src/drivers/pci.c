#include "pci.h"
#include "../klog.h"
#include "../io.h"
#include "../list.h"
#include "../kmalloc.h"
#include "../string.h"

static struct PCI {
   struct LinkedList pci_dev_list;
} pci_state = {
   .pci_dev_list = LINKED_LIST_INIT(pci_state.pci_dev_list, struct PCIDevice, list)
};

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func,
      uint8_t offset)
{
   uint32_t address, lbus  = (uint32_t)bus, lslot = (uint32_t)slot;
   uint32_t lfunc = (uint32_t)func;

   /* create configuration address */
   address = (uint32_t)((lbus << 16) | (lslot << 11) |
            (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

   /* write out the address and return the data */
   outl(PCI_CONFIG_ADDR, address);
   return inl(PCI_CONFIG_DATA);
}

uint16_t pci_read_config_vendor(uint8_t bus, uint8_t slot, uint8_t func)
{
   uint32_t tmp;

   tmp = pci_read_config(bus, slot, func, 0);
   return (uint16_t)(tmp & 0xffff);
}

void pci_read_config_header(struct PCIConfigHeader *hdr,
      uint8_t bus, uint8_t slot, uint8_t func)
{
   uint32_t *rhdr = (uint32_t *)hdr;
   uint8_t offset;

   for (offset = 0; offset < 4; offset++)
      rhdr[offset] = pci_read_config(bus, slot, func, offset*4);
}

int PCI_read_config_header_0(struct PCIDevice *dev, struct PCIConfigHeader_0 *hdr)
{
   uint32_t *rhdr = (uint32_t *)hdr;
   uint8_t offset;

   if (dev->hdr.header_type != 0) {
      klog(KLOG_LEVEL_WARN, "attempting to read pci config header of incorrect type");
      return -1;
   }

   hdr->hdr = dev->hdr;

   for (offset = 4; offset < 10; offset++)
      rhdr[offset] = pci_read_config(dev->bus, dev->slot, dev->func, offset*4);
   return 0;
}

int pci_enum_device(uint8_t bus, uint8_t slot)
{
   struct PCIConfigHeader hdr;
   struct PCIDevice *new;
   uint8_t func;

   if (pci_read_config_vendor(bus, slot, 0) == 0xffff)
      return 0;

   for (func = 0; func < 4; func++) {
      pci_read_config_header(&hdr, bus, slot, func);
      if (hdr.vendor_id == 0xffff)
         return 1;
      new = kmalloc(sizeof(struct PCIDevice));
      if (!new)
         return -1;
      memset(new, 0, sizeof(struct PCIDevice));
      new->bus = bus;
      new->slot = slot;
      new->func = func;
      new->hdr = hdr;
      new->id.class = hdr.class;
      new->id.sub_class = hdr.sub_class;
      new->id.device_id = hdr.device_id;
      new->id.vendor_id = hdr.vendor_id;
      LL_enqueue(&pci_state.pci_dev_list, new);
      klog(KLOG_LEVEL_INFO, "enumerated PCI device with class 0x%X on bus %d slot %d func %d",
            new->hdr.class, new->bus, new->slot, new->func);
   }

   return 1;
}

int PCI_register(struct PCIDriver *driver)
{
   struct PCIDevice *curr;

   LL_for_each(&pci_state.pci_dev_list, curr) {
      if (curr->id.class == driver->id.class &&
            curr->id.sub_class == driver->id.sub_class) {
         driver->probe(curr);
      }
   }

   return 0;
}

int PCI_enum()
{
   uint16_t bus;
   uint8_t slot;

   for (bus = 0; bus < 256; bus++)
      for (slot = 0; slot < 32; slot++)
         pci_enum_device(bus, slot);

   return 0;
}