#include <io.h>
#include <smi.h>
#include <pci.h>
#include <reg-82801b.h>
#include "vga-overlay.h"

void __firstrun_start() {
	dologf("NetWatch running");

	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
	pci_write16(0, 31, 2, 0xC0, 0);
	pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
	pci_write16(0, 31, 4, 0xC0, 0);

	/* Turn on the SMIs we want */
	outb(0x830, inb(0x830) | ICH2_SMI_EN_SWSMI_TMR_EN);
	outb(0x848, ICH2_DEVTRAP_EN_KBC_TRP_EN);
	smi_enable();
}

