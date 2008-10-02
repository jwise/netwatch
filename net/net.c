#include <pci.h>
#include <pci-bother.h>
#include <output.h>
#include "net.h"

static struct nic *_nic = 0x0;

extern struct pci_driver a3c90x_driver;

void eth_poll()
{
	if (_nic)
		_nic->poll(_nic, 0);
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
	if (pci_probe_driver(a3c90x_driver))
		outputf("found 3c90x, hopefully!");
}
