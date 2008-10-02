#ifndef _PCI_BOTHER_H
#define _PCI_BOTHER_H

#include <pci.h>

#define MAX_BOTHERS 5

extern int pci_bother_add(pci_dev_t *dev);
extern void pci_bother_all();
extern void pci_unbother_all();

#endif
