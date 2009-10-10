/* smm-open-ich7.c
 * SMRAM open utility for Intel 82865 northbridge
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */

#include "reg-82865.h"

void main()
{
	unsigned char smramc, esmramc;
	unsigned int toud;
	
	smramc = pci_read8(0, 0, 0, SMRAMC);
	esmramc = pci_read8(0, 0, 0, ESMRAMC);
	
	toud = (pci_read16(0, 0, 0, 0xC4) >> 3) << 19;
	printf("Usable DRAM: %d bytes\n", toud);
	
	printf("SMRAMC: %02x\n", smramc);
	printf("ESMRAMC: %02x\n", smramc);
	if (smramc & SMRAMC_LOCK)
	{
		printf("SMRAM is locked, cannot load anything :-(\n");
		return;
	}
}
