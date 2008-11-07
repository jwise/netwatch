#include <io.h>
#include <smram.h>
#include <video_defines.h>
#include <minilib.h>
#include <smi.h>
#include <pci-bother.h>
#include <serial.h>
#include "../net/net.h"
#include "vga-overlay.h"

#include "pagetable.h"

unsigned int counter = 0;
unsigned int lastctr = 0;
unsigned long pcisave = 0;
unsigned char vgasave = 0;

void set_cr0(unsigned int);

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


void smi_entry(void)
{
	char statstr[512];
	outb(0x80, 0x0B);
	pcisave = inl(0xCF8);
	vgasave = inb(0x3D4);
	outb(0x80, 0x1B);
/*
	pci_unbother_all();
 */
	counter++;
	outb(0x80, 0x2B);
	sprintf(statstr, "15-412! %08x %08x", smi_status(), counter);
	outb(0x80, 0x3B);
	strblit(statstr, 0, 0);
	outb(0x80, 0x4B);
	
	serial_init();
	dolog("wee!");
	
	/*
	eth_poll();
	*/
	
	if (inl(0x840) & 0x1000)
	{
	/*
		pci_dump();
	*/
		outl(0x840, 0x1100);
		outl(0x840, 0x0100);
	}


	smi_poll();
/*	
	pci_bother_all();
 */
	outl(0xCF8, pcisave);
	outb(0x3D4, vgasave);
}

void timer_handler(smi_event_t ev)
{
        static unsigned int ticks = 0;

        smi_disable_event(SMI_EVENT_FAST_TIMER);
        smi_enable_event(SMI_EVENT_FAST_TIMER);

        outb(0x80, (ticks++) & 0xFF);

        outlog();
}


void __firstrun_start() {
	smram_state_t smram;
	
	smram = smram_save_state();
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	smi_disable();
	outb(0x80, 0x41);

        smi_register_handler(SMI_EVENT_FAST_TIMER, timer_handler);
        smi_enable_event(SMI_EVENT_FAST_TIMER);

        smi_enable();
}

