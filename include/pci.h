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

/* Hardware-agnostic functions implemented by pci.c */
typedef enum {
	PCI_BAR_NONE = 0,
	PCI_BAR_MEMORY32,
	PCI_BAR_MEMORY64,
	PCI_BAR_IO
} pci_bar_type_t;

typedef struct pci_bar {
	pci_bar_type_t type;
	unsigned char prefetchable;
	unsigned long addr;
} pci_bar_t;

typedef struct pci_dev {
	unsigned short vid, did;
	int bus, dev, fn;
	pci_bar_t bars[6];
} pci_dev_t;

typedef int (*pci_probe_fn_t)(pci_dev_t *, void *data);

void pci_bus_enum();
int pci_probe(pci_probe_fn_t probe, void *data);

typedef struct pci_id {
	unsigned short vid, did;
	const char *name, *friendlyname;
} pci_id_t;

#define PCI_ROM(a,b,c,d) {(a),(b),(c),(d)}

typedef struct pci_driver {
	const char *name;
	pci_probe_fn_t probe;
	pci_id_t *ids;
	int id_count;
} pci_driver_t;

int pci_probe_driver(pci_driver_t driver);

#endif
