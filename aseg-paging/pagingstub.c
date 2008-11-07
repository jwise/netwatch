#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include <serial.h>
#include "../net/net.h"
#include "vga-overlay.h"

#include "vm_flags.h"
#include "pagetable.h"

void set_cr0(unsigned int);

extern int entry_initialized;
extern int _bss, _bssend, _end;
void smi_entry();
#define get_cr0() \
    ({ \
        register unsigned int _temp__; \
        asm volatile("mov %%cr0, %0" : "=r" (_temp__)); \
        _temp__; \
    })


#define set_cr3(value) \
    { \
        register unsigned int _temp__ = (value); \
        asm volatile("mov %0, %%cr3" : : "r" (_temp__)); \
     }
#define	CR0_PG	0x80000000

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

	for (i = 0; i < 32; i++) { 
		pagetable[0xA0 + i] = (0xA0000 + i * 0x1000) | MAP_FLAGS;
	}

	/* Map 0x200000 to TSEG */
	for (i = 0; i < 128; i++) {
		pagetable[0x200 + i] = (0x1FF80000 + i * 0x1000) | MAP_FLAGS;
	}

	/* Map 0x300000 -> 0x200000, so we can copy our code out of
	 * RAM the first time around */
	for (i = 0; i < 256; i++) {
	/*	pagetable[0x300 + i] = (0x200000 + i * 0x1000) | MAP_FLAGS;
	 */
		pagetable[0x200 + i] = (0x200000 + i * 0x1000) | MAP_FLAGS;
	}

	outb(0x80, 0x57);
	return pagedirectory;
}

void c_entry(void)
{
//	unsigned char *bp;

	outb(0x80, 0x41);
	char * pagedir = pt_setup(0xA0000);
	outb(0x80, 0x43);
	set_cr3((int)pagedir);
	outb(0x80, 0xA5);

	/* Turn paging on */
	set_cr0(get_cr0() | CR0_PG);
	serial_init();
	serial_tx('A');
	outb(0x80, 0xAA);
	if (!entry_initialized) {
		serial_tx('B');
		outb(0x80, 0xAB);
/*
		for (bp = (void *)0x200000; (void *)bp < (void *)&_bss; bp++)
			*bp = *(bp + 0x100000);

		for (bp = (void *)&_bss; (void *)bp < (void *)&_bssend; bp++)
			*bp = 0;
			*/
	}

	smi_entry();
}
