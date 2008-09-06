#include <io.h>
#include <inttypes.h>

static void __pci_config(int bus, int slot, int fn, int addr)
{
	outl(0xCF8, 0x80000000ULL | (bus << 16) | (slot << 11) | (fn << 8) | addr);
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
	return inw(0xCFC);
}

uint8_t pci_read8(int bus, int slot, int fn, int addr)
{
	__pci_config(bus, slot, fn, addr);
	return inb(0xCFC);
}
