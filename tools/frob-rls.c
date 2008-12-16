/* frob-rls.c
 * GBL_RLS flag manipulator.
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <sys/io.h>
#include <reg-82801b.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static uint16_t _get_PMBASE()
{
	static long pmbase = -1;

	if (pmbase == -1) 
		pmbase = pci_read32(ICH2_LPC_BUS, ICH2_LPC_DEV, ICH2_LPC_FN, ICH2_LPC_PCI_PMBASE) & ICH2_PMBASE_MASK;

	return pmbase;
}

typedef struct {
	uint32_t signature;
	uint32_t type;
	uint8_t data[];
} packet_t;

extern unsigned int poke(unsigned long addr, unsigned long * value);

int main(int argc, char **argv)
{
	unsigned int res;
	packet_t * packet = (packet_t *)memalign(4096, sizeof(packet_t));

	packet->signature = 0x1BADD00D;
	packet->type = 0xAA;
	strcpy(packet->data, "hello, world!");

	if (iopl(3) < 0)
	{
		perror("iopl");
		return 1;
	}
	
	res = poke(_get_PMBASE() + 0x04, (void *)packet);
	printf("returned %p\n", res);

	if (res == 42) {
		int i;
		for (i = 0; i < 96; i++)
			printf("%s\n", packet->data + i * 41);
	}
}
