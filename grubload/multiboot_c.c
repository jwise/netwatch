#include "console.h"
#include <io.h>

extern char _binary_realmode_bin_start[];
extern int _binary_realmode_bin_size;

struct mb_info
{
	unsigned long flags;
	unsigned long mem_lower, mem_upper;
	unsigned long boot_dev;
	char *cmdline;
	unsigned long mod_cnt;
	struct mod_info *mods;
};

struct mod_info
{
	void *mod_start;
	void *mod_end;
	char *mod_string;
	void *reserved;
};

void c_start(unsigned int magic, struct mb_info *wee)
{
	unsigned short *grubptr = 0x7CFE;
	unsigned char smramc;
	int i;
	
	void (*realmode)() = 0x4000;
	
	puts("Magic is: ");
	puthex(magic);
	puts("\nMultiboot header is: ");
	puthex(wee);
	puts("\n");
	show_cursor();
	
	puts("Grubptr is: ");
	puthex(*grubptr);
	puts("\n");


	for (i = 0; i < wee->mod_cnt; i++)
	{
		puts("Module:\n");
		puts("  Start: "); puthex(wee->mods[i].mod_start); puts("\n");
		puts("  Size: "); puthex(wee->mods[i].mod_end - wee->mods[i].mod_start); puts("\n");
		puts("  Name: "); puts(wee->mods[i].mod_string); puts("\n");
	}

	if (wee->mod_cnt != 1)
	{
		puts("Expected exactly one module; cannot continue.\n");
		while(1) asm("hlt");
	}

	puts("Current USB state is: "); puthex(pci_read16(0, 31, 2, 0xC0)); puts(" "); puthex(pci_read16(0, 31, 4, 0xC0)); puts("\n");
	puts("Current SMI state is: "); puthex(inl(0x830)); puts("\n");
	puts("Current SMRAMC state is: "); puthex(pci_read8(0, 0, 0, 0x70)); puts("\n");
	
	outl(0x830, inl(0x830) & ~0x2001);	/* turn off SMIs */
	
	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
	pci_write16(0, 31, 2, 0xC0, 0);
	pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
	pci_write16(0, 31, 4, 0xC0, 0);
	smramc = pci_read8(0, 0, 0, 0x70);
	pci_write8(0, 0, 0, 0x70, (smramc & 0xF0) | 0x04);
	load_elf(wee->mods[0].mod_start, wee->mods[0].mod_end - wee->mods[0].mod_start);
	pci_write8(0, 0, 0, 0x70, smramc);
	outb(0x830, inb(0x830) | 0x41);	/* turn on the SMIs we want */
	
	puts("Waiting for a bit before returning to real mode...");
	for (i=0; i<0x500000; i++)
	{
		if ((i % 0x100000) == 0)
			puts(".");
		inb(0x80);
	}
	puts("\n");

	puts("Now returning to real mode.\n");	
	memcpy(0x4000, _binary_realmode_bin_start, (int)&_binary_realmode_bin_size);
	realmode();	// goodbye!
}
