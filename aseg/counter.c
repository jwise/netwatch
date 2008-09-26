#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include "vga-overlay.h"
#include "packet.h"

unsigned int counter = 0;
unsigned long pcisave;
unsigned char vgasave;

void pci_dump() {
	unsigned long cts;
	static int curdev = 0;	/* 0 if kbd, 1 if mouse */
		
	cts = inl(0x84C);
	
	outl(0x848, 0x0);
	outl(0x840, 0x0);
	switch(cts&0xF0000)
	{
	case 0x20000:
	{
		unsigned char b;
		b = inb(cts & 0xFFFF);
		dologf("READ: %08x (%02x)", cts, b);
		if ((cts & 0xFFFF) == 0x64)
			curdev = (b & 0x20) ? 1 : 0;
		if ((curdev == 0) && ((cts & 0xFFFF) == 0x60) && (b == 0x01))
			outb(0xCF9, 0x4);
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
	
	outl(0x848, 0x1000);
	outl(0x840, 0x0100);
}

void timer_handler(smi_event_t ev)
{
	static unsigned int ticks = 0;
	
	smi_disable_event(SMI_EVENT_FAST_TIMER);
	smi_enable_event(SMI_EVENT_FAST_TIMER);
	
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
	} else {
		*(unsigned long*)0xAFFD4 = 0x2BADD00D;
	}
}

void smi_entry(void)
{
	char statstr[512];
	
	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	
	counter++;
	sprintf(statstr, "15-412! %08x %08x", smi_status(), counter);
	strblit(statstr, 0, 0);
	
	if (inl(0x840) & 0x1000)
	{
		pci_dump();
		outl(0x840, 0x1100);
		outl(0x840, 0x0100);
	}

	smi_poll();
	
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
}

