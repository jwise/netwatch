#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include <serial.h>
#include "../net/net.h"
#include "vga-overlay.h"

unsigned int counter = 0;
unsigned long pcisave = 0;
unsigned char vgasave = 0;

void smi_entry(void)
{
	char statstr[512];

	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	pci_unbother_all();

	counter++;
	sprintf(statstr, "NetWatch! %08x %08x", smi_status(), counter);
	strblit(statstr, 0, 0, 0);
	
	serial_init();
	eth_poll();
	
	if (inl(0x840) & 0x1000)
	{
	/*
		pci_dump();
	*/
		outl(0x840, 0x1100);
		outl(0x840, 0x0100);
	}


	smi_poll();
	
	pci_bother_all();
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
}

extern void timer_handler(smi_event_t ev);
extern void kbc_handler(smi_event_t ev);
extern void gbl_rls_handler(smi_event_t ev);

void __firstrun_stub() {

        /* Try really hard to shut up USB_LEGKEY. */
        pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
        pci_write16(0, 31, 2, 0xC0, 0);
        pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
        pci_write16(0, 31, 4, 0xC0, 0);

        /* Turn on the SMIs we want */
        smi_disable();

        smi_register_handler(SMI_EVENT_FAST_TIMER, timer_handler);
        smi_enable_event(SMI_EVENT_FAST_TIMER);

        smi_register_handler(SMI_EVENT_DEVTRAP_KBC, kbc_handler);
        smi_enable_event(SMI_EVENT_DEVTRAP_KBC);

        smi_register_handler(SMI_EVENT_GBL_RLS, gbl_rls_handler);
        smi_enable_event(SMI_EVENT_GBL_RLS);

        smi_enable();
}
