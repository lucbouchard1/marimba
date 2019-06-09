#ifndef __PCI_H__
#define __PCI_H__

#include "../types.h"

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

int PCI_enum();

#endif