/* smi.c
 * System management interrupt dispatch routines for ICH2 southbridge
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree. 
 *
 */


#include <smi.h>
#include <pci.h>
#include <io.h>
#include <stdint.h>
#include <vga-overlay.h>
#include <reg-82801b.h>
#include <output.h>

static smi_handler_t _handlers[SMI_EVENT_MAX] = {0};

static uint16_t _get_PMBASE()
{
	static long pmbase = -1;
	
	if (pmbase == -1)	/* Memoize it so that we don't have to hit PCI so often. */
		pmbase = pci_read32(ICH2_LPC_BUS, ICH2_LPC_DEV, ICH2_LPC_FN, ICH2_LPC_PCI_PMBASE) & ICH2_PMBASE_MASK;
	
	return pmbase;
}

void smi_disable()
{
	unsigned short smi_en = _get_PMBASE() + ICH2_PMBASE_SMI_EN;
	outl(smi_en, inl(smi_en) & ~ICH2_SMI_EN_GBL_SMI_EN);
	
	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(ICH2_USB0_BUS, ICH2_USB0_DEV, ICH2_USB0_FN, ICH2_USB_LEGKEY, 0x0);
	pci_write16(ICH2_USB0_BUS, ICH2_USB0_DEV, ICH2_USB0_FN, ICH2_USB_LEGKEY,
		pci_read16(ICH2_USB0_BUS, ICH2_USB0_DEV, ICH2_USB0_FN, ICH2_USB_LEGKEY));
	pci_write16(ICH2_USB1_BUS, ICH2_USB1_DEV, ICH2_USB1_FN, ICH2_USB_LEGKEY, 0x0);
	pci_write16(ICH2_USB1_BUS, ICH2_USB1_DEV, ICH2_USB1_FN, ICH2_USB_LEGKEY,
		pci_read16(ICH2_USB1_BUS, ICH2_USB1_DEV, ICH2_USB1_FN, ICH2_USB_LEGKEY));
	
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
		if (_handlers[SMI_EVENT_GBL_RLS] == SMI_HANDLER_NONE)
			output("Unhandled: BIOS_STS");
		else if (_handlers[SMI_EVENT_GBL_RLS] != SMI_HANDLER_IGNORE)
			_handlers[SMI_EVENT_GBL_RLS](SMI_EVENT_GBL_RLS);
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_BIOS_STS);
	}
	
	if (sts & ICH2_SMI_STS_LEGACY_USB_STS)
	{
		output("Unhandled: LEGACY_USB_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_LEGACY_USB_STS);
	}
	
	if (sts & ICH2_SMI_STS_SLP_SMI_STS)
	{
		output("Unhandled: SLP_SMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SLP_SMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_APM_STS)
	{
		output("Unhandled: APM_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_APM_STS);
	}
	
	if (sts & ICH2_SMI_STS_SWSMI_TMR_STS)	// Ack it, then request another.
	{
		if (_handlers[SMI_EVENT_FAST_TIMER] == SMI_HANDLER_NONE)
			output("Unhandled: SWSMI_TMR_STS");
		else if (_handlers[SMI_EVENT_FAST_TIMER] != SMI_HANDLER_IGNORE)
			_handlers[SMI_EVENT_FAST_TIMER](SMI_EVENT_FAST_TIMER);
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SWSMI_TMR_STS);
	}
	
	if (sts & ICH2_SMI_STS_PM1_STS_REG)
	{
		unsigned short pm1_sts = inw(_get_PMBASE() + ICH2_PMBASE_PM1_STS);
		unsigned short pm1_en = inw(_get_PMBASE() + ICH2_PMBASE_PM1_EN);
		
		pm1_sts &= pm1_en;
		if (pm1_sts & ICH2_PM1_STS_RTC_STS)
		{
			output("Unhandled: PM1_STS: RTC_STS");
			outw(_get_PMBASE() + ICH2_PMBASE_PM1_STS, ICH2_PM1_STS_RTC_STS);
		}
		
		if (pm1_sts & ICH2_PM1_STS_PWRBTN_STS)
		{
			if (_handlers[SMI_EVENT_PWRBTN] == SMI_HANDLER_NONE)
				output("Unhandled: PM1_STS: PWRBTN_STS");
			else if (_handlers[SMI_EVENT_FAST_TIMER] != SMI_HANDLER_IGNORE)
				_handlers[SMI_EVENT_PWRBTN](SMI_EVENT_PWRBTN);
			outw(_get_PMBASE() + ICH2_PMBASE_PM1_STS, ICH2_PM1_STS_PWRBTN_STS);
		}
		
		if (pm1_sts & ICH2_PM1_STS_GBL_STS)
		{
			output("Unhandled: PM1_STS: GBL_STS");
			outw(_get_PMBASE() + ICH2_PMBASE_PM1_STS, ICH2_PM1_STS_GBL_STS);
		}
		
		if (pm1_sts & ICH2_PM1_STS_TMROF_STS)
		{
			output("Unhandled: PM1_STS: TMROF_STS");
			outw(_get_PMBASE() + ICH2_PMBASE_PM1_STS, ICH2_PM1_STS_TMROF_STS);
		}
		
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_PM1_STS_REG);
	}
	
	if (sts & ICH2_SMI_STS_GPE0_STS)
	{
		/* XXX -- trawl through GPE0_STS to see what happened */
		output("XXX Unhandled: GPE0_STS (expect lockup)");
	}
	
	if (sts & ICH2_SMI_STS_GPE1_STS)
	{
		/* XXX -- trawl through GPE1_STS to see what happened */
		output("XXX Unhandled: GPE1_STS (expect lockup)");
	}
	
	if (sts & ICH2_SMI_STS_MCSMI_STS)
	{
		output("Unhandled: MCSMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_MCSMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_DEVMON_STS)
	{
		unsigned short mon_smi = inw(_get_PMBASE() + ICH2_PMBASE_MON_SMI);
		unsigned long devact_sts = inl(_get_PMBASE() + ICH2_PMBASE_DEVACT_STS);
		unsigned long devtrap_en = inl(_get_PMBASE() + ICH2_PMBASE_DEVTRAP_EN);
		
		if (devact_sts & ICH2_DEVACT_STS_KBC_ACT_STS)
		{
			if (_handlers[SMI_EVENT_DEVTRAP_KBC] == SMI_HANDLER_NONE)
				output("Unhandled: DEVACT_KBC_ACT_STS");
			else if (_handlers[SMI_EVENT_DEVTRAP_KBC] != SMI_HANDLER_IGNORE)
				_handlers[SMI_EVENT_DEVTRAP_KBC](SMI_EVENT_DEVTRAP_KBC);
			outl(_get_PMBASE() + ICH2_PMBASE_DEVACT_STS, ICH2_DEVACT_STS_KBC_ACT_STS);
		}
		
		/* Refresh register cache so that we can print unhandleds as needed. */
		mon_smi = inw(_get_PMBASE() + ICH2_PMBASE_MON_SMI);
		devact_sts = inl(_get_PMBASE() + ICH2_PMBASE_DEVACT_STS);
		devtrap_en = inl(_get_PMBASE() + ICH2_PMBASE_DEVTRAP_EN);
		
		if (((mon_smi & 0x0F00) >> 8) & ((mon_smi & 0xF000) >> 12))
			outputf("Unhandled: MON_SMI (%04x)", mon_smi);
		if (devact_sts & devtrap_en)
			outputf("Unhandled: DEVTRAP (%08x)", devact_sts & devtrap_en);
	}
	
	if (sts & ICH2_SMI_STS_TCO_STS)
	{
		output("Unhandled: TCO_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_TCO_STS);
	}
	
	if (sts & ICH2_SMI_STS_PERIODIC_STS)
	{
		output("Unhandled: PERIODIC_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_PERIODIC_STS);
	}
	
	if (sts & ICH2_SMI_STS_SERIRQ_SMI_STS)
	{
		output("Unhandled: SERIRQ_SMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SERIRQ_SMI_STS);
	}
	
	if (sts & ICH2_SMI_STS_SMBUS_SMI_STS)
	{
		output("Unhandled: SMBUS_SMI_STS");
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_STS, ICH2_SMI_STS_SMBUS_SMI_STS);
	}
	
	if (smi_status() & ~ICH2_SMI_STS_PM1_STS_REG)	/* Either the chipset is buggy, or we are. */
		outputf("WARN: couldn't clear SMI_STS! (%08x)", smi_status());
	
	outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
		inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) |
			ICH2_SMI_EN_EOS |
			ICH2_SMI_EN_GBL_SMI_EN);
}

int smi_register_handler(smi_event_t ev, smi_handler_t hnd)
{
	if (ev >= SMI_EVENT_MAX)
		return -1;
	_handlers[ev] = hnd;
	return 0;
}

int smi_enable_event(smi_event_t ev)
{
	switch(ev)
	{
	case SMI_EVENT_FAST_TIMER:
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) |
				ICH2_SMI_EN_SWSMI_TMR_EN);
		return 0;
	case SMI_EVENT_DEVTRAP_KBC:
		outl(_get_PMBASE() + ICH2_PMBASE_DEVTRAP_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_DEVTRAP_EN) |
				ICH2_DEVTRAP_EN_KBC_TRP_EN);
		return 0;
	case SMI_EVENT_GBL_RLS:
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) |
				ICH2_SMI_EN_BIOS_EN);
		return 0;
	case SMI_EVENT_PWRBTN:
		outl(_get_PMBASE() + ICH2_PMBASE_PM1_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_PM1_EN) |
				ICH2_PM1_EN_PWRBTN_EN);
		return 0;
	default:
		return -1;
	}
}

int smi_disable_event(smi_event_t ev)
{
	switch(ev)
	{
	case SMI_EVENT_FAST_TIMER:
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) &
				~ICH2_SMI_EN_SWSMI_TMR_EN);
		return 0;
	case SMI_EVENT_DEVTRAP_KBC:
		outl(_get_PMBASE() + ICH2_PMBASE_DEVTRAP_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_DEVTRAP_EN) &
				~ICH2_DEVTRAP_EN_KBC_TRP_EN);
		return 0;
	case SMI_EVENT_GBL_RLS:
		outl(_get_PMBASE() + ICH2_PMBASE_SMI_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_SMI_EN) &
				~ICH2_SMI_EN_BIOS_EN);
		return 0;
	case SMI_EVENT_PWRBTN:
		outl(_get_PMBASE() + ICH2_PMBASE_PM1_EN,
			inl(_get_PMBASE() + ICH2_PMBASE_PM1_EN) &
				~ICH2_PM1_EN_PWRBTN_EN);
		return 0;
	default:
		return -1;
	}
}
