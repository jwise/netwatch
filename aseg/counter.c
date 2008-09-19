#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>

unsigned int counter = 0;
unsigned long pcisave;
unsigned char vgasave;
char thestr[512];
char logents[4][41] = {{0}};

#define VRAM_BASE		0xA0000UL
#define TEXT_CONSOLE_OFFSET	0x18000UL 

#define TEXT_CONSOLE_BASE	(VRAM_BASE + TEXT_CONSOLE_OFFSET)

#define COLOR			0x1F

unsigned char vga_read(unsigned char idx)
{
	outb(CRTC_IDX_REG, idx);
	return inb(CRTC_DATA_REG);
}

char * vga_base()
{
	return (char *) (
		TEXT_CONSOLE_BASE
		| (((unsigned int) vga_read(CRTC_START_ADDR_LSB_IDX)) << 9)
		| (((unsigned int) vga_read(CRTC_START_ADDR_MSB_IDX)) << 1)
	);
}

void strblit(char *src, int row, int col)
{
	char *destp = vga_base() + row * 80 * 2 + col * 2;
	smram_state_t old_state = smram_save_state();

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	
	while (*src)
	{
		*(destp++) = *(src++);
		*(destp++) = COLOR;
	}

	smram_restore_state(old_state);
}

void outlog()
{
	int y, x;
	char *basep = vga_base();

	smram_state_t old_state = smram_save_state();

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);

	for (y = 0; y < 4; y++)
		for (x = 40; x < 80; x++)
		{
			basep[y*80*2+x*2] = ' ';
			basep[y*80*2+x*2+1] = 0x1F;
		}

	smram_restore_state(old_state);

	for (y = 0; y < 4; y++)
		strblit(logents[y], y, 40);
}

void dolog(char *s)
{
	memmove(logents[0], logents[1], sizeof(logents[0])*3);
	strcpy(logents[3], s);
}

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

