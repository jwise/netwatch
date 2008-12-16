/* pagingstub.c
 * Paging enable routines
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */


#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include <serial.h>
#include <output.h>
#include "traps.h"
#include "../net/net.h"
#include "vga-overlay.h"

extern void smi_init();
#include "vm_flags.h"

extern void smi_entry();
void set_cr0(unsigned int);
void ps_switch_stack (void (*call)(), int stack);

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

#define get_cr4() \
    ({ \
        register unsigned int _temp__; \
        asm volatile("mov %%cr4, %0" : "=r" (_temp__)); \
        _temp__; \
    })
#define set_cr4(value) \
    { \
        register unsigned int _temp__ = (value); \
        asm volatile("mov %0, %%cr4" : : "r" (_temp__)); \
     }

#define	CR0_PG	0x80000000
#define CR4_PSE	0x00000010

#define MAP_FLAGS	(PTE_PRESENT | PTE_READ_WRITE)

static int initialized = 0;
static int paging_enb = 0;
static unsigned long *pd;

extern int _bss, _bssend, _end;

static unsigned long curmapped = 0xFFFFFFFF;

unsigned long v2p(void *virt)
{
	unsigned long _virt = (unsigned long)virt;
	
	if (!paging_enb)
		return _virt;
	
	unsigned long pde = ((unsigned long *)p2v((unsigned long)pd))[PDE_FOR(_virt)];
	unsigned long pte;
	
	if (!(pde & PTE_PRESENT))
		return 0xFFFFFFFF;
	
	if (pde & PDE_PAGE_SIZE)
		return ADDR_12_MASK(pde) + (_virt & 0x3FFFFF);
	
	pte = ((unsigned long *)p2v(ADDR_12_MASK(pde)))[PTE_FOR(_virt)];
	if (!(pte & PTE_PRESENT))
		return 0xFFFFFFFF;
	return (pte & ~0xFFF) + (_virt & 0xFFF);
}

void *p2v(unsigned long phys)
{
	if (!paging_enb)
		return (void*)phys;
	
	if (phys >= 0xA0000 && phys < 0xC0000)
		return (void*)phys;
	if (phys >= 0x1FF82000 && phys < 0x20000000)
		return (void*)(phys - 0x1FF82000 + 0x200000);
	if (phys >= 0x1FF80000 && phys < 0x1FF82000)
		return (void*)(phys - 0x1FF80000 + 0x1F0000);
		
	if ((phys & ~0xFFF) != curmapped)	/* If it's not mapped, map it in. */
	{
		curmapped = phys & ~0xFFF;
		addmap(0x4000, curmapped);
		asm volatile("invlpg 0x4000");
	}
	return (void*)(0x4000 + (phys & 0xFFF));
}

int addmap(unsigned long vaddr, unsigned long paddr)
{
	unsigned long pde = ((unsigned long *)p2v((unsigned long)pd))[PDE_FOR(vaddr)];
	unsigned long *pt;
	
	if (!(pde & PTE_PRESENT))
		return -1;
	
	pt = (unsigned long *)p2v(ADDR_12_MASK(pde));
	pt[PTE_FOR(vaddr)] = paddr | PTE_PRESENT | PTE_READ_WRITE;
	
	return 0;
}

int addmap_4m(unsigned long vaddr, unsigned long paddr)
{
	/* PDE_PAGE_SIZE = (1 << 7) */
	((unsigned long *)p2v((unsigned long)pd))[PDE_FOR(vaddr)] =
		paddr | PDE_PRESENT | PDE_READ_WRITE | PDE_PAGE_SIZE;
	
	return 0;
}

void *demap(unsigned long client_pd, unsigned long vaddr)
{
	unsigned long pde = ((unsigned long *)p2v(client_pd))[PDE_FOR(vaddr)];
	unsigned long pte;
	
	if (!(pde & PTE_PRESENT))
		return (void*)0x0;
	pte = ((unsigned long *)p2v(ADDR_12_MASK(pde)))[PTE_FOR(vaddr)];
	if (!(pte & PTE_PRESENT))
		return (void*)0x0;
	return p2v((pte & ~0xFFF) + (vaddr & 0xFFF));
}

static void pt_setup(int tseg_start, int tseg_size) {
	int i;

	/* The page directory and page table live at TSEG and TSEG + 0x1000,
	 * respectively. */
	unsigned long *pagedirectory = (unsigned long *) tseg_start;
	unsigned long *pagetable = (unsigned long *) (tseg_start + 0x1000);
	
	pd = pagedirectory;

	/* Clear out the page directory except for one entry pointing to the
	 * page table, and clear the page table entirely. */
	pagedirectory[0] = (tseg_start + 0x1000) | PTE_PRESENT | PTE_READ_WRITE;
	for (i = 1; i < 1024; i++)
		pagedirectory[i] = 0;

	for (i = 0; i < 1024; i++)
		pagetable[i] = 0;

	/* Map 0x0A0000:0x0BFFFF to itself. */
	for (i = 0; i < 32; i++)
		addmap(0xA0000 + i * 0x1000, 0xA0000 + i * 0x1000);

	/* Map 0x200000:0x300000 to TSEG data */
	for (i = 0; i < 256; i++)
		addmap(0x200000 + i * 0x1000, tseg_start + (i + 2) * 0x1000);

	/* Map 0x300000:0x400000 to 0x200000, so we can copy our code out of
	 * RAM the first time around */
	for (i = 0; i < 256; i++)
		addmap(0x300000 + i * 0x1000, 0x200000 + i * 0x1000);

	/* Map 0x1F0000:0x1F2000 to TSEG paging info */
	for (i = 0; i < 2; i++)
		addmap(0x1F0000 + i * 0x1000, tseg_start + i * 0x1000);
}

void init_and_run(void)
{
	if (!initialized)
	{
		smi_init();
		initialized = 1;
	}
	
	smi_entry();
}

void c_entry(void)
{
	paging_enb = 0;

	outb(0x80, 0x01);	
	if (!initialized)
		pt_setup(0x1FF80000, 0x80000);
	outb(0x80, 0x02);
		
	/* Enable paging. */
	set_cr3((unsigned long)pd);
	set_cr4(get_cr4() | CR4_PSE);	/* ITT, we 4MByte page. */ 
	set_cr0(get_cr0() | CR0_PG);
	outb(0x80, 0x03);
	paging_enb = 1;

	/* If this is the first goround, copy in data. */
	if (!initialized)
	{
		unsigned char *p;
		
		outb(0x80, 0x04);
		for (p = (void *)0x200000; (void *)p < (void *)&_bss; p++)
			*p = *(p + 0x100000);
		for (p = (void *)&_bss; (void *)p < (void *)&_bssend; p++)
			*p = 0;
		outb(0x80, 0x05);
		
		/* Only now is it safe to call other functions. */
		serial_init();
		dolog("Evacuation to TSEG complete.");
	}
	
	outb(0x80, 0x06);

	traps_install();
	
	outb(0x80, 0x07);

	ps_switch_stack(init_and_run, 0x270000);
	outb(0x80, 0xFA);
}
