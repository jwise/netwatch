#include "reg-82815.h"
#include <pci.h>
#include <smram.h>

#ifndef __RAW__

static unsigned long memsz[] = {
	0,			// 0
	32*1024*1024,		// 1
	32*1024*1024,		// 2
	48*1024*1024,		// 3
	64*1024*1024,		// 4
	64*1024*1024,		// 5
	96*1024*1024,		// 6
	128*1024*1024,		// 7
	128*1024*1024,		// 8
	128*1024*1024,		// 9
	128*1024*1024,		// A
	192*1024*1024,		// B
	256*1024*1024,		// C
	256*1024*1024,		// D
	256*1024*1024,		// E
	512*1024*1024		// F
};

void smram_aseg_dump(void) {

	unsigned char smramc, drp, drp2;
	unsigned int tom = 0;
	int usmm, lsmm;

	smramc = pci_read8(0, 0, 0, SMRAMC);
	drp = pci_read8(0, 0, 0, DRP);
	drp2 = pci_read8(0, 0, 0, DRP2);

	printf("SMRAMC: %02x\n", smramc);

	tom += memsz[drp & 0xF];
	tom += memsz[drp >> 4];
	tom += memsz[drp2 & 0xF];
	
	printf("Top of DRAM: %08x\n", tom);
	
	usmm = (smramc >> 4) & 0x3;
	lsmm = (smramc >> 2) & 0x3;
	
	switch (usmm)
	{
	case 0:
		printf("TSEG and HSEG both off\n");
		break;
	case 1:
		printf("TSEG off, HSEG %s\n", lsmm ? "off" : "on");
		break;
	case 2:
		printf("TSEG 512KB (%08x - %08x), HSEG %s\n",
			tom - 512 * 1024, tom - 1, lsmm ? "off" : "on");
		break;
	case 3:
		printf("TSEG 1MB (%08x - %08x), HSEG %s\n",
			tom - 1 * 1024 * 1024, tom - 1, lsmm ? "off" : "on");
		break;
	}
	
	switch (lsmm)
	{
	case 0:
		printf("ABSEG disabled\n");
		break;
	case 1:
		printf("ABSEG enabled as system RAM\n");
		break;
	case 2:
		printf("ABSEG enabled for SMM code only\n");
		break;
	case 3:
		printf("ABSEG enabled for both SMM code and data\n");
		break;
	}
}
#endif

int smram_locked()
{
	unsigned char smramc = pci_read8(0, 0, 0, SMRAMC);
	
	return (smramc & SMRAMC_LOCK) ? 1 : 0;
}

smram_state_t smram_save_state()
{
	return pci_read8(0, 0, 0, SMRAMC);
}

void smram_restore_state(smram_state_t state)
{
	pci_write8(0, 0, 0, SMRAMC, state); 
}

int smram_aseg_set_state (int open) {
	unsigned char smramc;

	if (smram_locked())
		return -1;
		
	smramc = pci_read8(0, 0, 0, SMRAMC);

	switch (open)
	{
	case SMRAM_ASEG_CLOSED:
		smramc = (smramc & 0xF0) | 0x00;
		break;
	case SMRAM_ASEG_OPEN:
		smramc = (smramc & 0xF0) | 0x04;
		break;
	case SMRAM_ASEG_SMMCODE:
		smramc = (smramc & 0xF0) | 0x08;
		break;
	case SMRAM_ASEG_SMMONLY:
		smramc = (smramc & 0xF0) | 0x0C;
		break;
	default:
		return -1;
	}

	pci_write8(0, 0, 0, SMRAMC, smramc);

	return 0;
}
