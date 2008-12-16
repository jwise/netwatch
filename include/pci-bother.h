/* pci-bother.h
 * Definitions for Bothering a PCI device
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#ifndef _PCI_BOTHER_H
#define _PCI_BOTHER_H

#include <pci.h>

#define MAX_BOTHERS 5

extern int pci_bother_add(pci_dev_t *dev);
extern void pci_bother_all();
extern void pci_unbother_all();

#endif
