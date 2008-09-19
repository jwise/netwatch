#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>

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
	
	switch(cts&0xF0000)
	{
	case 0x20000:
	{
		unsigned char b;
		strcpy(s, "READxxxxxxxxxxxxxxxx");
		tohex(s+4, cts);
		b = inb(cts & 0xFFFF);
		tohex(s+12, b);
		if ((cts & 0xFFFF) == 0x64)
			curdev = (b & 0x20) ? 1 : 0;
		if ((curdev == 0) && ((cts & 0xFFFF) == 0x60) && (b == 0x01))
			outb(0xCF9, 0x4);
		dolog(s);
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
}

void __start (void)
{
	static int first = 1;
	
	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	
	if (first)
	{
		first = 0;
		dolog("NetWatch running...");
	}

	counter++;
	outb(0x80, (counter & 0xFF));
	
	strcpy(thestr, "15-412! xxxxxxxx xxxxxxxx");
	tohex(thestr + 8, inl(0x0834));
	tohex(thestr + 17, counter);
	strblit(thestr, 0, 0);
	
	if (inl(0x834) & 0x20)
		dolog("Warning: unhandled APM access");
	if (inl(0x834) & 0x1000)
	{
		if (inl(0x844) & 0x1000)	/* devact_sts */
		{
			pci_dump();
			outl(0x848, 0x1000);
			outl(0x844, 0x1000);
		}
	}
	if (inl(0x834) & 0x4000)
		dolog("Long periodic timer");
	if (inl(0x840) & 0x1000)
	{
		dolog("Caught device monitor trap");
		pci_dump();
		outl(0x840, 0x1100);
		outl(0x840, 0x0100);
	}
	if (inl(0x834) & ~(0x4160))
	{
		char s[40];
		strcpy(s, "Unknown: xxxxxxxx");
		tohex(s + 9, inl(0x834) & ~(0x140));
		dolog(s);
	}


	outlog();
	
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
	
	outl(0x848, 0x1000);
	outl(0x834, /*0x40*/0xFFFF);	// ack the periodic IRQ
	outb(0x830, (inb(0x830) | 0x2) & ~0x40);
	outb(0x830, inb(0x830) | 0x40);
	
}

