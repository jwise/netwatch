#include "console.h"
#include "loader.h"

#include <minilib.h>
#include <io.h>
#include <smram.h>
#include <multiboot.h>
#include <smi.h>
#include <pci.h>

#define INFO_SIGNATURE 0x5754454E

extern char _binary_realmode_bin_start[];
extern int _binary_realmode_bin_size;

struct info_section
{
	unsigned int signature;
	void (*firstrun)();
};

void panic(const char *msg)
{
	puts("PANIC: ");
	puts(msg);
	puts("\nSystem halted\n");
	while(1) { __asm__("hlt"); }
}

void dolog(char *s)
{
	/* little shim -- need to unify logging */
	puts(s);
}

void c_start(unsigned int magic, struct mb_info *mbinfo)
{
	struct mod_info *mods = mbinfo->mods;
	smram_state_t old_smramc;
	struct info_section * info;
	int i;
	
	void (*realmode)() = (void (*)()) 0x4000;
	
	show_cursor();
	puts("NetWatch loader\n");
	
	if (magic != MULTIBOOT_LOADER_MAGIC)
		panic("Bootloader was not multiboot compliant; cannot continue.");
	
	for (i = 0; i < mbinfo->mod_cnt; i++)
	{
		puts("Module found:\n");
		puts("  Start: "); puthex((unsigned long) mods[i].mod_start); puts("\n");
		puts("  Size: "); puthex((unsigned long)mods[i].mod_end - (unsigned long)mods[i].mod_start); puts("\n");
		puts("  Name: "); puts(mods[i].mod_string); puts("\n");
	}

	if (mbinfo->mod_cnt != 1)
		panic("Expected exactly one module; cannot continue.");

	puts("Current USB state is: "); puthex(pci_read16(0, 31, 2, 0xC0)); puts(" "); puthex(pci_read16(0, 31, 4, 0xC0)); puts("\n");
	puts("Current SMI state is: "); puthex(inl(0x830)); puts("\n");
	puts("Current SMRAMC state is: "); puthex(pci_read8(0, 0, 0, 0x70)); puts("\n");
	
	smi_disable();
	
	/* Try really hard to shut up USB_LEGKEY. */
	pci_write16(0, 31, 2, 0xC0, pci_read16(0, 31, 2, 0xC0));
	pci_write16(0, 31, 2, 0xC0, 0);
	pci_write16(0, 31, 4, 0xC0, pci_read16(0, 31, 4, 0xC0));
	pci_write16(0, 31, 4, 0xC0, 0);

	/* Open the SMRAM aperture and load our ELF. */
	old_smramc = smram_save_state();

	if (smram_aseg_set_state(SMRAM_ASEG_OPEN) != 0)
		panic("Opening SMRAM failed; cannot load ELF.");

	load_elf(mods[0].mod_start, (unsigned long)mods[0].mod_end - (unsigned long)mods[0].mod_start);

	info = (struct info_section *)0x10000;
	if (info->signature != INFO_SIGNATURE)
	{
		smram_restore_state(old_smramc);		/* Restore so that video ram is touchable again. */
		panic("Info section signature mismatch.");
	}

	info->firstrun();
	smram_restore_state(old_smramc);

	puts("Waiting for a bit before returning to real mode...");
	for (i=0; i<0x500000; i++)
	{
		if ((i % 0x100000) == 0)
			puts(".");
		inb(0x80);
	}
	puts("\n");

	puts("Now returning to real mode.\n");	
	memcpy((void *)0x4000, _binary_realmode_bin_start, (int)&_binary_realmode_bin_size);
	realmode();	// goodbye!
}
