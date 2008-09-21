#include <smi.h>
#include <pci.h>
#include <io.h>
#include <stdint.h>
#include <reg-82801b.h>

uint16_t _get_PMBASE()
{
	return pci_read32(ICH2_LPC_BUS, ICH2_LPC_DEV, ICH2_LPC_FN, ICH2_LPC_PCI_PMBASE) & ICH2_PMBASE_MASK;
}

void smi_disable()
{
	unsigned short smi_en = _get_PMBASE() + ICH2_PMBASE_SMI_EN;
	outl(smi_en, inl(smi_en) & ~ICH2_SMI_EN_GBL_SMI_EN);
}

void smi_enable()
{
	unsigned short smi_en = _get_PMBASE() + ICH2_PMBASE_SMI_EN;
	outl(smi_en, inl(smi_en) | ICH2_SMI_EN_GBL_SMI_EN);
}

unsigned long smi_status()
{
	unsigned short smi_sts = _get_PMBASE() + ICH2_PMBASE_SMI_STS;
	return inl(smi_sts);
}

