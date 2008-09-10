#include "console.h"

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

	if ((wee->mod_cnt != 1) || (strcmp(wee->mods[0].mod_string, "aseg.elf")))
	{
		puts("Expected 1 module called aseg.elf.\n");
		while(1) asm("hlt");
	}

	load_elf(wee->mods[0].mod_start, wee->mods[0].mod_end - wee->mods[0].mod_start);

	puts("Now returning to real mode.\n");	
	memcpy(0x4000, _binary_realmode_bin_start, (int)&_binary_realmode_bin_size);
	realmode();	// goodbye!
}
