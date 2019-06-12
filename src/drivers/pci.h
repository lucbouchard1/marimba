#ifndef __PCI_H__
#define __PCI_H__

#include "../types.h"
#include "../fs.h"

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

struct PCIDevId {
   uint16_t vendor_id;
   uint16_t device_id;
   uint8_t class;
   uint8_t sub_class;
};

struct PCIConfigHeader {
   uint16_t vendor_id;
   uint16_t device_id;
   uint16_t command;
   uint16_t status;
   uint8_t rev;
   uint8_t prog_if;
   uint8_t sub_class;
   uint8_t class;
   uint8_t cache_line_size;
   uint8_t latency_timer;
   uint8_t header_type;
   uint8_t bist;
} __attribute__((packed));

struct PCIConfigHeader_0 {
   struct PCIConfigHeader hdr;
   uint32_t bar0;
   uint32_t bar1;
   uint32_t bar2;
   uint32_t bar3;
   uint32_t bar4;
   uint32_t bar5;
};

struct PCIDriver;

struct PCIDevice {
   struct ListHeader list;
   struct PCIDevId id;
   uint8_t bus;
   uint8_t slot;
   uint8_t func;
   struct PCIConfigHeader hdr;
};

struct PCIDriver {
   struct PCIDevId id;
   int (*probe)(struct PCIDevice *device);
};

int PCI_enum();
int PCI_register(struct PCIDriver *driver);
int PCI_read_config_header_0(struct PCIDevice *dev, struct PCIConfigHeader_0 *hdr);

#endif