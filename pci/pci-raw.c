/* pci-raw.c
 * Raw PCI implementation
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <io.h>
#include <stdint.h>
#include <pci.h>

static void __pci_config(int bus, int slot, int fn, int addr)
{
	outl(0xCF8, 0x80000000ULL | (bus << 16) | (slot << 11) | (fn << 8) | (addr & ~3));
}

void pci_write32(int bus, int slot, int fn, int addr, uint32_t data)
{
	__pci_config(bus, slot, fn, addr);
	outl(0xCFC, data);
}

void pci_write16(int bus, int slot, int fn, int addr, uint16_t data)
{
	__pci_config(bus, slot, fn, addr);
	outw(0xCFC, data);
}

void pci_write8(int bus, int slot, int fn, int addr, uint8_t data)
{
	__pci_config(bus, slot, fn, addr);
	outb(0xCFC, data);
}

uint32_t pci_read32(int bus, int slot, int fn, int addr)
{
	__pci_config(bus, slot, fn, addr);
	return inl(0xCFC);
}

uint16_t pci_read16(int bus, int slot, int fn, int addr)
{
	__pci_config(bus, slot, fn, addr);
	return inw(0xCFC + (addr & 2));
}

uint8_t pci_read8(int bus, int slot, int fn, int addr)
{
	__pci_config(bus, slot, fn, addr);
	return inb(0xCFC + (addr & 3));
}
