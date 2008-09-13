#ifndef PCI_H
#define PCI_H

#include <stdint.h>

/* General PCI functions. This is implemented by pci-linux.c and pci-raw.c; the
 * former uses Linux's /proc/bus/pci interface for access from userspace, while
 * the latter accesses the PCI hardware directly.
 */

void pci_write32(int bus, int slot, int fn, int addr, uint32_t data);
void pci_write16(int bus, int slot, int fn, int addr, uint16_t data);
void pci_write8(int bus, int slot, int fn, int addr, uint8_t data);

uint32_t pci_read32(int bus, int slot, int fn, int addr);
uint16_t pci_read16(int bus, int slot, int fn, int addr);
uint8_t pci_read8(int bus, int slot, int fn, int addr);

#endif
