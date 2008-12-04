#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include <serial.h>
#include <fb.h>
#include <output.h>
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
	
	serial_init();
	
	if (fb)
        	fb->getvmode(fb->priv);

	counter++;
	sprintf(statstr, "NetWatch! %08x %08x", smi_status(), counter);
	strblit(statstr, 0, 0, 0);
	
	/* wee! */
	if (fb && !fb->curmode.text)
	{
		int ass[][2] = {
			      {1,0},       {4,0},{5,0},{6,0}, {8,0},{9,0},{10,0},
			{0,1},      {2,1}, {4,1},             {8,1},
			{0,2},{1,2},{2,2}, {4,2},{5,2},{6,2}, {8,2},{9,2},{10,2},
			{0,3},      {2,3},             {6,3},             {10,3},
			{0,4},      {2,4}, {4,4},{5,4},{6,4}, {8,4},{9,4},{10,4},
			{-1,-1}
		};
		int p;
		for (p = 0; ass[p][0] != -1; p++)
		{
			int x, y;
			for (y = 0; y < 8; y++)
				for (x = 0; x < 8; x++)
				{
					unsigned long *a =
						(unsigned long*) fb->fbaddr +
						((y+ass[p][1]*8) * fb->curmode.xres + ass[p][0]*8 + x);
                                        *a = 0xFF0000FF;
				}
		}
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
