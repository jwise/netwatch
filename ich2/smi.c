#include <smi.h>
#include <pci.h>
#include <io.h>
#include <stdint.h>
#include <vga-overlay.h>
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

void smi_poll()
{
	unsigned long sts = smi_status();
	
	if (sts & ICH2_SMI_STS_BIOS_STS)
	{
		dolog("Unhandled: BIOS_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_BIOS_STS);
	}
	
	if (sts & ICH2_SMI_STS_LEGACY_USB_STS)
	{
		dolog("Unhandled: LEGACY_USB_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_LEGACY_USB_STS);
	}
	
	if (sts & ICH2_SMI_STS_SLP_SMI_STS)
	{
		dolog("Unhandled: SLP_SMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SLP_SMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_APM_STS)
	{
		dolog("Unhandled: APM_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SLP_SMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_SWSMI_TMR_STS)	// Ack it, then request another.
	{
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SLP_SMI_STS);
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) & ~ICH2_SMI_EN_SWSMI_TMR_EN);
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) | ICH2_SMI_EN_SWSMI_TMR_EN);
	}
	
	if (sts & ICH2_SMI_STS_PM1_STS_REG)
	{
		/* XXX -- trawl through PMBASE+00h to see what happened */
	}
	
	if (sts & ICH2_SMI_STS_GPE0_STS)
	{
		/* XXX -- trawl through GPE0_STS to see what happened */
	}
	
	if (sts & ICH2_SMI_STS_MCSMI_STS)
	{
		dolog("Unhandled: MCSMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_MCSMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_DEVMON_STS)
	{
		/* XXX -- trawl through DEVx_TRAP_STS to see what happened */
		/* XXX -- trawl through DEVTRAP_STS to see what happened */
	}
	
	if (sts & ICH2_SMI_STS_TCO_STS)
	{
		dolog("Unhandled: TCO_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_TCO_STS);
	}
	
	if (sts & ICH2_SMI_STS_PERIODIC_STS)
	{
		dolog("Unhandled: PERIODIC_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_PERIODIC_STS);
	}
	
	if (sts & ICH2_SMI_STS_SERIRQ_SMI_STS)
	{
		dolog("Unhandled: SERIRQ_SMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SERIRQ_SMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_SMBUS_SMI_STS)
	{
		dolog("Unhandled: SMBUS_SMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SMBUS_SMI_STS);
	}
	
	if (smi_status())
		dolog("WARNING: failed to clear SMI_STS!");
	
	outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
		inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) |
			ICH2_SMI_EN_EOS |
			ICH2_SMI_EN_GBL_SMI_EN);
}
