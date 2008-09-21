#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>

char thestr[512];

#include "vga-overlay.h"

unsigned int counter = 0;
unsigned long pcisave;
unsigned char vgasave;

void pci_dump() {
	char s[40];
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
		
		strcpy(s, "WRITxxxxxxxxxxxxxxxx");
		b = *(unsigned char*)0xAFFD0 /* EAX */;
		tohex(s+4, cts);
		tohex(s+12, b);
		dolog(s);
		outb(cts & 0xFFFF, b);
		break;
	}
	default:
		dolog("Unhandled PCI cycle");
	}
	
	outl(0x848, 0x1000);
	outl(0x840, 0x0100);
}

void smi_entry(void)
{
	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	
	counter++;
	outb(0x80, (counter & 0xFF));
	
	sprintf(thestr, "15-412! %08x %08x", smi_status(), counter);
	strblit(thestr, 0, 0);
	
	if (inl(0x834) & 0x20)
		dolog("Warning: unhandled APM access");
	if (inl(0x834) & 0x1000)
	{
		if (inl(0x844) & 0x1000)	/* devact_sts */
		{
			pci_dump();
			outl(0x844, 0x1000);	/* ack it */
		}
	}
	if (inl(0x834) & 0x4000)
		dolog("Long periodic timer");
	if (inl(0x840) & 0x1000)
	{
		pci_dump();
		outl(0x840, 0x1100);
		outl(0x840, 0x0100);
	}
	if (inl(0x834) & ~(0x4160))
		dologf("Unknown: %08x", inl(0x834) & ~(0x140));

	outlog();
	
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
	
	outl(0x848, 0x1000);
	outl(0x834, /*0x40*/0xFFFF);	// ack the periodic IRQ
	outb(0x830, (inb(0x830) | 0x2) & ~0x40);
	outb(0x830, inb(0x830) | 0x40);
	
}

