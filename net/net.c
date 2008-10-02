#include <pci.h>
#include <output.h>
#include "3c905.h"

static int found = 0, _bus, _dev, _fn;

static int bother_3c905(pci_dev_t *dev, void *nutrinus)
{
	if (dev->vid == 0x10B7 || dev->did == 0x9200)
	{
		outputf("Found a 3c905 to bother");
		
		_bus = dev->bus;
		_dev = dev->dev;
		_fn = dev->fn;
		found = 1;
		
		return 1;
	}
	return 0;
}

void eth_poll()
{
	if (!found)
		return;
		
	pci_write16(_bus, _dev, _fn, 0x04, 0xFF);
	
	pci_write16(_bus, _dev, _fn, 0x04, 0x00);
}

void eth_init()
{
	pci_probe(bother_3c905, 0x0);
}
