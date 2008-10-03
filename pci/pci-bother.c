#include <pci.h>
#include <pci-bother.h>

struct pci_bother {
	int bus, dev, fn;
	unsigned short origstate;
	unsigned long origbars[6];
};

static struct pci_bother bothers[MAX_BOTHERS];
static int nbothers = 0;

int pci_bother_add(pci_dev_t *dev)
{
	int i;
	
	if (nbothers == MAX_BOTHERS)
		return -1;
	
	bothers[nbothers].bus = dev->bus;
	bothers[nbothers].dev = dev->dev;
	bothers[nbothers].fn = dev->fn;
	
	bothers[nbothers].origstate = pci_read16(dev->bus, dev->dev, dev->fn, 0x04);
	for (i = 0; i < 6; i++)
		bothers[nbothers].origbars[i] = pci_read32(dev->bus, dev->dev, dev->fn, 0x10 + i * 4);
	
	nbothers++;
	
	return 0;
}

void pci_bother_all()
{
	int i, j;
	
	for (i = 0; i < nbothers; i++)
	{
		pci_write16(bothers[i].bus, bothers[i].dev, bothers[i].fn, 0x04, 0x0);
		for (j = 0; j < 6; j++)
			pci_write32(bothers[i].bus, bothers[i].dev, bothers[i].fn, 0x10 + j * 4, 0);
	}
}

void pci_unbother_all()
{
	int i, j;
	
	for (i = 0; i < nbothers; i++)
	{
		pci_write16(bothers[i].bus, bothers[i].dev, bothers[i].fn, 0x04, bothers[i].origstate);
		for (j = 0; j < 6; j++)
			pci_write32(bothers[i].bus, bothers[i].dev, bothers[i].fn, 0x10 + j * 4, bothers[i].origbars[j]);
	}
}
