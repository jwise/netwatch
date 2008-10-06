#include <pci.h>
#include <pci-bother.h>
#include <output.h>
#include "net.h"

static struct nic *_nic = 0x0;

extern struct pci_driver a3c90x_driver;

static char test[1024] = {0};

void eth_poll()
{
	int i;
	static int c;
	
	if (!_nic)
		return;

	_nic->poll(_nic, 0);
	
	if ((c++) % 30)
		return;
	
	for (i = 0; i < 1024; i++)
	{
		switch(i%5)
		{
		case 0: test[i] = 'H'; break;
		case 1: test[i] = 'A'; break;
		case 2: test[i] = 'R'; break;
		case 3: test[i] = 'B'; break;
		case 4: test[i] = 'L'; break;
		}
	}
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
