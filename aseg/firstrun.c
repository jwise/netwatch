#include <io.h>
#include <smi.h>
#include <pci.h>
#include <reg-82801b.h>
#include <output.h>
#include "vga-overlay.h"
#include <smram.h>

extern int _bss, _bssend;

extern void timer_handler(smi_event_t ev);
extern void kbc_handler(smi_event_t ev);
extern void gbl_rls_handler(smi_event_t ev);

int bother_3c905(pci_dev_t *dev)
{
	if (dev->vid == 0x10B7 || dev->did == 0x9200)
	{
		int bar;
		
		outputf("Found a 3c905 to bother");
		pci_write16(dev->bus, dev->dev, dev->fn, 0x04, 0x00);
		
		for (bar = 0; bar < 6; bar++)
			if (dev->bars[bar].type != PCI_BAR_NONE)
				pci_write32(dev->bus, dev->dev, dev->fn, 0x10 + bar*4, 0);
		
		return 1;
	}
	return 0;
}

void __firstrun_start() {
	unsigned char *bp;
	smram_state_t smram;
	
	smram = smram_save_state();
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	
	for (bp = (void *)&_bss; (void *)bp < (void *)&_bssend; bp++)
		*bp = 0;
	
	outputf("NetWatch running");

	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
	pci_write16(0, 31, 2, 0xC0, 0);
	pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
	pci_write16(0, 31, 4, 0xC0, 0);

	/* Turn on the SMIs we want */
	smi_disable();
	
	pci_probe(bother_3c905);
	
	smi_register_handler(SMI_EVENT_FAST_TIMER, timer_handler);
	smi_enable_event(SMI_EVENT_FAST_TIMER);
	
	smi_register_handler(SMI_EVENT_DEVTRAP_KBC, kbc_handler);
	smi_enable_event(SMI_EVENT_DEVTRAP_KBC);
	
	smi_register_handler(SMI_EVENT_GBL_RLS, gbl_rls_handler);
	smi_enable_event(SMI_EVENT_GBL_RLS);

	smi_enable();
	
	smram_restore_state(smram);
}

