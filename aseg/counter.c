#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include "../net/net.h"
#include "vga-overlay.h"
#include "packet.h"
#include "keyboard.h"

unsigned int counter = 0;
unsigned int lastctr = 0;
unsigned long pcisave;
unsigned char vgasave;
static int curdev = 0;	/* 0 if kbd, 1 if mouse */

static void cause_kbd_irq()
{
	outl(0x844, 0x0);
	outl(0x848, 0x0);
	while (inb(0x64) & 0x1)
		inb(0x60);
	outb(0x60, 0xee);	/* Cause an IRQ. */
	while (inb(0x60) != 0xEE)
		;
}

void pci_dump() {
	unsigned long cts;
		
	cts = inl(0x84C);
	
	outl(0x840, 0x0);
	outl(0x848, 0x0);
	switch(cts&0xF0000)
	{
	case 0x20000:
	{
		unsigned char b;
		
		switch (cts & 0xFFFF)
		{
		case 0x64:
			/* Read the real hardware and mask in our OBF if need be. */
			b = inb(0x64);
			if (kbd_has_injected_scancode())
			{
				dologf("OS wants to know; we have data");
				lastctr = counter;
				b |= 0x01;
				b &= ~0x20;	/* no mouse for you! */
				curdev = 0;
			} else 
				curdev = (b & 0x20) ? 1 : 0;
			*(unsigned char*)0xAFFD0 /* EAX */ = b;
			break;
		case 0x60:
			if (kbd_has_injected_scancode())
			{
				b = kbd_get_injected_scancode();
				lastctr = counter;
				while (inb(0x64) & 0x1)
					inb(0x60);
			} else
				b = inb(0x60);
			if ((curdev == 0) && (b == 0x01)) {	/* Escape */
				outb(0xCF9, 0x4);	/* Reboot */
				return;
			}
			
			/* If there is more nus to come, generate another IRQ. */
			if (kbd_has_injected_scancode())
				cause_kbd_irq();
			
			*(unsigned char*)0xAFFD0 /* EAX */ = b;
			break;
		}

		*(unsigned char*)0xAFFD0 /* EAX */ = b;
		break;
	}
	case 0x30000:
	{
		unsigned char b;
		
		b = *(unsigned char*)0xAFFD0 /* EAX */;
		dologf("WRITE: %08x (%02x)", cts, b);
		outb(cts & 0xFFFF, b);
		break;
	}
	default:
		dolog("Unhandled PCI cycle");
	}
	
	outl(0x844, 0x1000);
	outl(0x848, 0x1000);
}

void timer_handler(smi_event_t ev)
{
	static unsigned int ticks = 0;
	
	smi_disable_event(SMI_EVENT_FAST_TIMER);
	smi_enable_event(SMI_EVENT_FAST_TIMER);
	
	if (kbd_has_injected_scancode() && (counter > (lastctr + 2)))
	{
		smi_disable_event(SMI_EVENT_DEVTRAP_KBC);
		dolog("Kicking timer");
		cause_kbd_irq();
		smi_enable_event(SMI_EVENT_DEVTRAP_KBC);
	}
	
	outb(0x80, (ticks++) & 0xFF);
	
	outlog();
}

void kbc_handler(smi_event_t ev)
{
	pci_dump();
}

void gbl_rls_handler(smi_event_t ev)
{
	unsigned long ecx;
	
	ecx = *(unsigned long*)0xAFFD4;

	packet_t * packet = check_packet(ecx);
	if (!packet)
	{
		dologf("WARN: bad packet at %08x", ecx);
		return;
	}

	dologf("Got packet: type %08x", packet->type);

	if (packet->type == 42) {
		dump_log((char *)packet->data);
		*(unsigned long*)0xAFFD4 = 42;
	} else if (packet->type == 0xAA) {
		kbd_inject_key('A');
	} else {
		*(unsigned long*)0xAFFD4 = 0x2BADD00D;
	}
}

void smi_entry(void)
{
	char statstr[512];
	
	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	pci_unbother_all();
	
	counter++;
	sprintf(statstr, "15-412! %08x %08x", smi_status(), counter);
	strblit(statstr, 0, 0);
	
	eth_poll();
	
	if (inl(0x840) & 0x1000)
	{
		pci_dump();
		outl(0x840, 0x1100);
		outl(0x840, 0x0100);
	}

	smi_poll();
	
	pci_bother_all();
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
}

