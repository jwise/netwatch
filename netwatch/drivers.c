/* drivers.c
 * PCI driver table
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <pci.h>

extern struct pci_driver a3c90x_driver;
extern struct pci_driver tnt2_driver;

struct pci_driver *drivers[] =
{
	&a3c90x_driver,
	&tnt2_driver,
	0
};

