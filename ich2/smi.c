#include <smi.h>
#include <pci.h>
#include <io.h>
#include <stdint.h>

uint16_t _get_PMBASE()
{
	return pci_read32(0, 0, 0, 0x40) & 0xFF80;
}

void smi_disable()
{
	unsigned short smi_en = _get_PMBASE() + 0x30;
	outl(smi_en, inl(smi_en) & ~0x0001);
}

void smi_enable()
{
	unsigned short smi_en = _get_PMBASE() + 0x30;
	outl(smi_en, inl(smi_en) | 0x0001);
}

unsigned long smi_status()
{
	unsigned short smi_sts = _get_PMBASE() + 0x34;
	return inl(smi_sts);
}
