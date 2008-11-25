#include <io.h>
#include <smi.h>
#include <pci.h>
#include <reg-82801b.h>
#include <output.h>
#include "vga-overlay.h"
#include <smram.h>
#include "../net/net.h"

extern int _bss, _bssend;

extern void timer_handler(smi_event_t ev);
extern void kbc_handler(smi_event_t ev);
extern void gbl_rls_handler(smi_event_t ev);

extern pci_driver_t *drivers[];

void smi_init() {
	smram_state_t smram;
	pci_driver_t **driver;
	
	smram = smram_save_state();
	smram_tseg_set_state(SMRAM_TSEG_OPEN);

	outputf("NetWatch running");

	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
	pci_write16(0, 31, 2, 0xC0, 0);
	pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
	pci_write16(0, 31, 4, 0xC0, 0);

	/* Turn on the SMIs we want */
	smi_disable();
	
	eth_init();
	
	/* After everything is initialized, load drivers. */
	for (driver = drivers; *driver; driver++)
	{
		outputf("Probing driver: %s", (*driver)->name);
		if (pci_probe_driver(*driver))
			output("Found a card");
	}
	outputf("Driver probe complete");

	smi_register_handler(SMI_EVENT_FAST_TIMER, timer_handler);
	smi_enable_event(SMI_EVENT_FAST_TIMER);

	smi_register_handler(SMI_EVENT_DEVTRAP_KBC, kbc_handler);
	smi_enable_event(SMI_EVENT_DEVTRAP_KBC);
	
	smi_register_handler(SMI_EVENT_GBL_RLS, gbl_rls_handler);
	smi_enable_event(SMI_EVENT_GBL_RLS);

	smi_enable();
	
	vga_flush_imm(1);
	
	smram_restore_state(smram);
}

