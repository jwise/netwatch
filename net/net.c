#include <pci.h>
#include <pci-bother.h>
#include <output.h>
#include "net.h"

static int bother_3c905(pci_dev_t *dev, void *nutrinus)
{
	if (dev->vid == 0x10B7 || dev->did == 0x9200)
	{
		outputf("Found a 3c905 to bother");
		
		pci_bother_add(dev);
		return 1;
	}
	return 0;
}

void eth_poll()
{
	/* ... */
}

void eth_init()
{
	pci_probe(bother_3c905, 0x0);
}
