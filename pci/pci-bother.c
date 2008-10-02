#include <pci.h>
#include <pci-bother.h>

struct pci_bother {
	int bus, dev, fn;
	unsigned short origstate;
};

static struct pci_bother bothers[MAX_BOTHERS];
static int nbothers = 0;

int pci_bother_add(pci_dev_t *dev)
{
	if (nbothers == MAX_BOTHERS)
		return -1;
	
	bothers[nbothers].bus = dev->bus;
	bothers[nbothers].dev = dev->dev;
	bothers[nbothers].fn = dev->fn;
	
	bothers[nbothers].origstate = pci_read16(dev->bus, dev->dev, dev->fn, 0x04);
	
	nbothers++;
	
	return 0;
}

void pci_bother_all()
{
	int i;
	
	for (i = 0; i < nbothers; i++)
		pci_write16(bothers[i].bus, bothers[i].dev, bothers[i].fn, 0x04, 0x0);
}

void pci_unbother_all()
{
	int i;
	
	for (i = 0; i < nbothers; i++)
		pci_write16(bothers[i].bus, bothers[i].dev, bothers[i].fn, 0x04, bothers[i].origstate);
}
