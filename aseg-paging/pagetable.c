#include "vm_flags.h"
#include <io.h>

#define MAP_FLAGS	(PTE_PRESENT | PTE_READ_WRITE)

void * pt_setup(int smbase) {
	int i;
	outb(0x80, 0x51);

	/* The page directory and page table live at SMBASE and SMBASE + 0x1000,
	 * respectively; clear them. */
	int * pagedirectory = (int *) smbase;
	int * pagetable = (int *) (smbase + 0x1000);

	/* Clear out the page directory except for one entry pointing to the
	 * page table, and clear the page table entirely. */
	outb(0x80, 0x52);
	pagedirectory[0] = (smbase + 0x1000) | PTE_PRESENT | PTE_READ_WRITE;
	outb(0x80, 0x53);
	for (i = 1; i < 1024; i++)
	{
		pagedirectory[i] = 0;
	}

	outb(0x80, 0x54);
	for (i = 0; i < 1024; i++)
	{
		pagetable[i] = 0;
	}
	outb(0x80, 0x55);

	/* The page at 0x10000 - 0x10FFF points to the SMI entry point,
	 * SMBASE + 0x8000. */
	pagetable[16] = (0x8000 + smbase) | MAP_FLAGS;

	/* 0x11000 to 0x1EFFF map to the rest of ASEG up to SMBASE + 0xF000;
	 * the page containing the saved state is not mappped to our code
	 * region.  */

	for (i = 0; i < 8; i++)
	{
		pagetable[17 + i] = (i * 0x1000 + smbase) | MAP_FLAGS;
	}

	for (i = 0; i < 6; i++)
	{
		pagetable[25 + i] = (smbase + 0x9000 + i * 0x1000) | MAP_FLAGS;
	}

	outb(0x80, 0x56);
	/* Map 0xA8000 to itself. */
	pagetable[0xA8] = 0xA8000 | MAP_FLAGS;
	pagetable[0xA9] = 0xA9000 | MAP_FLAGS;

	/* No TSEG yet. */

	outb(0x80, 0x57);
	return pagedirectory;
}
