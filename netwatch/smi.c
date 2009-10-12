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

unsigned long lastentry = 0;
unsigned long lastlength = 0;

unsigned long rdtsc()
{
	unsigned long tsc;
	asm volatile ("mov %%cr4, %%eax;"
	     "and $~2, %%eax;"
	     "mov %%eax, %%cr4;"
	     "rdtsc" : "=a"(tsc) : : "edx");
	return tsc;
}

void smi_entry(void)
{
	char statstr[512];
	unsigned long entrytime;

	/* Reenable caching on SMRAM. */
	WRMSR(0x202, (RDMSR(0x202) & ~(0xFFULL)) | 0x06ULL);

	entrytime = rdtsc();

	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	pci_unbother_all();
	
	serial_init();

	if (fb)
        	fb->getvmode(fb->priv);

	counter++;
	if (!fb || fb->curmode.text)
	{
		int totcyc, pct;
		
		if (entrytime < lastentry)
			totcyc = entrytime + (0xFFFFFFFFUL - lastentry) + 1;
		else
			totcyc = entrytime - lastentry;
		if (totcyc == 0)
			totcyc = 1;	/* argh */
		totcyc /= 1000;
		if (totcyc == 0)
			totcyc = 1;
		
		pct = lastlength / totcyc;
		
        	sprintf(statstr, "NetWatch! %08x %08x, %2d.%d%%", smi_status(), counter, pct/10, pct%10);
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
	
	lastentry = entrytime;
	entrytime = rdtsc();
	if (entrytime < lastentry)
		lastlength = entrytime + (0xFFFFFFFFUL - lastentry) + 1;
	else
		lastlength = entrytime - lastentry;
	
	/* Disable caching on SMRAM again, to prevent the user from whacking us. */
	WRMSR(0x202, RDMSR(0x202) & ~(0xFFULL));
}

extern void timer_handler(smi_event_t ev);
extern void kbc_handler(smi_event_t ev);
extern void gbl_rls_handler(smi_event_t ev);

void __firstrun_stub()
{
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
