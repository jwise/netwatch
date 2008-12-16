/* smm-open-ich2.c
 * SMRAM control utility for ICH2 southbridge
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */


#include "reg-82815.h"
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>

/***********************************************************************
 * DEPRECATED DEPRECATED DEPRECATED DEPRECATED DEPRECATED
 *
 * This should be replaced by ../util/smram-ich2 once it's verified to work properly.
 */

unsigned long memsz[] = {
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

void dump(unsigned char smramc, unsigned char drp, unsigned char drp2) {

	unsigned int tom = 0;
	int usmm, lsmm;

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

static struct option longopts[] = {
	{ "open",	no_argument,	NULL,	'o'	},
	{ "close",	no_argument,	NULL,	'c'	},
	{ "smram",	no_argument,	NULL,	's'	},
	{ "dump",	no_argument,	NULL,	'd'	},
	{ NULL,		0,		NULL,	0	}
};

#define OP_DUMP 1
#define OP_SET 2

void usage(int argc, char **argv)
{
	printf("Usage: %s [ --dump ] [ --open | --close | --smram=value ]\n",
		argv[0]);
	exit(1);
}

int main(int argc, char **argv)
{
	unsigned char smramc, drp, drp2;

	if (geteuid() != 0)
	{
		printf("This program must be run as root, dumbass.\n");
		return 1;
	}
	
	smramc = pci_read8(0, 0, 0, SMRAMC);
	drp = pci_read8(0, 0, 0, DRP);
	drp2 = pci_read8(0, 0, 0, DRP2);
	
	if (smramc & SMRAMC_LOCK)
	{
		printf("SMRAM is locked, cannot load anything :-(\n");
		return 1;
	}

	int ch;
	int op = 0;
	int new_smramc = smramc;

	while ((ch = getopt_long(argc, argv, "ocsd:", longopts, NULL)) != -1)
	{
		switch (ch)
		{
		case 's':
			if (op & OP_SET) usage(argc, argv);
			op |= OP_SET;
			new_smramc = strtoul(optarg, 0, 0);
			break;
		case 'o':
			if (op & OP_SET) usage(argc, argv);
			op |= OP_SET;
			/* Set LSMM to 01 (ABseg = system RAM) */
			new_smramc = (smramc & 0xF0) | 0x04;
			break;
		case 'c':
			if (op & OP_SET) usage(argc, argv);
			op |= OP_SET;
			/* Set LSMM to 11 (ABseg = SMM RAM) */
			new_smramc = (smramc & 0xF0) | 0x0C;
			break;
		case 'd':
			op |= OP_DUMP;
			break;
		default:
			usage(argc, argv);
		}
	}

	if (!op) usage(argc, argv);

	if (op & OP_DUMP)
	{
		dump(smramc, drp, drp2);
	}

	if (op & OP_SET)
	{
		pci_write8(0, 0, 0, SMRAMC, new_smramc);
		if (op & OP_DUMP)
		{
			printf("SMRAM set to 0x%02x\n", new_smramc);
		}
	}
	
	return 0;
}
