/* pci.c
 * Arbitrary PCI space reader/writer utility
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <pci.h>
#include <string.h>

int main(int argc, char **argv)
{
	unsigned int bus, dev, fn, addr;
	unsigned char type = 'b';
	unsigned int datum;
	
	if (iopl(3) < 0)
	{
		perror("iopl");
		return 1;
	}
	
	if ((argc < 5) || (argc > 7))
	{
	usage:
		printf("usage: %s bus dev fn addr [b|w|l [datum]]\n", argv[0]);
		return 2;
	}
	
	bus = strtoul(argv[1], NULL, 0);
	dev = strtoul(argv[2], NULL, 0);
	fn = strtoul(argv[3], NULL, 0);
	addr = strtoul(argv[4], NULL, 0);
	
	if (argc > 5)
		type = *argv[5];
	
	if (argc > 6)
	{
		datum = strtoul(argv[6], NULL, 0);
		switch (type)
		{
		case 'b':
			datum &= 0xFF;
			pci_write8(bus, dev, fn, addr, datum);
			printf("Wrote byte 0x%02x to %02x:%02x.%x[%02x]\n", datum, bus, dev, fn, addr);
			break;
		case 'w':
			datum &= 0xFFFF;
			pci_write16(bus, dev, fn, addr, datum);
			printf("Wrote word 0x%04x to %02x:%02x.%x[%02x]\n", datum, bus, dev, fn, addr);
			break;
		case 'l':
			pci_write32(bus, dev, fn, addr, datum);
			printf("Wrote long 0x%08x to %02x:%02x.%x[%02x]\n", datum, bus, dev, fn, addr);
			break;
		default:
			goto usage;
		}
	} else {
		switch(type)
		{
		case 'b':
			datum = pci_read8(bus, dev, fn, addr);
			printf("Read byte 0x%02x from %02x:%02x.%x[%02x]\n", datum, bus, dev, fn, addr);
			break;
		case 'w':
			datum = pci_read16(bus, dev, fn, addr);
			printf("Read word 0x%04x from %02x:%02x.%x[%02x]\n", datum, bus, dev, fn, addr);
			break;
		case 'l':
			datum = pci_read32(bus, dev, fn, addr);
			printf("Read long 0x%08x from %02x:%02x.%x[%02x]\n", datum, bus, dev, fn, addr);
			break;
		default:
			goto usage;
		}
	}
}
