/* smi.c
 * First-run SMI C entry point
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include <serial.h>
#include <fb.h>
#include <output.h>
#include <msr.h>
#include "../net/net.h"
#include "vga-overlay.h"

unsigned int counter = 0;
unsigned long pcisave = 0;
unsigned char vgasave = 0;

void smi_entry(void)
{
	char statstr[512];

	/* Reenable caching on SMRAM. */
	WRMSR(0x202, (RDMSR(0x202) & ~(0xFFULL)) | 0x06ULL);

	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	pci_unbother_all();
	
	serial_init();
	
	if (fb)
        	fb->getvmode(fb->priv);

	counter++;
	if (!fb || fb->curmode.text)
	{
        	sprintf(statstr, "NetWatch! %08x %08x", smi_status(), counter);
		strblit(statstr, 0, 0, 0);
	}
	
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
	
	/* Disable caching on SMRAM again, to prevent the user from whacking us. */
	WRMSR(0x202, RDMSR(0x202) & ~(0xFFULL));
}

extern void timer_handler(smi_event_t ev);
extern void kbc_handler(smi_event_t ev);
extern void gbl_rls_handler(smi_event_t ev);
extern void cs410_pwrbtn_handler(smi_event_t ev);

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
        
        smi_register_handler(SMI_EVENT_PWRBTN, cs410_pwrbtn_handler);
        smi_enable_event(SMI_EVENT_PWRBTN);

        smi_enable();
}
